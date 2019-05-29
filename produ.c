#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include <pthread.h>
#include <stdbool.h>
#include <semaphore.h> 

#include "linea.h"
#include "proceso.h"


// Variables globales
Linea * mem_address;
int * control_address; 
sem_t mutex;

/**
    Retorna un numero entero aleatorio entre [min, max]
*/
int getRandom(int min, int max){
    srand(time(0));
    int numero = (rand() % (max - min + 1)) + min;
    return numero;
}


/*
    Implementa la lógica del Algoritmo First Fit
*/
void * first_fit(void * arg){
    struct Proceso * actual;
    actual = (struct Proceso *)arg;

    // guardo estos porque la referencia a los parámetros cambia en tiempo de ejecución!
    int tamano = actual->tamano;
    int tiempo = actual->tiempo;

    int pos = 0;
    bool ocupado;
    bool insertado = false;

    // pide el semáforo
    sem_wait(&mutex);

    while(pos < (control_address[0] - tamano)){        //posicion menor que el tamaño de lineas
        ocupado = false;

        // para saltar más rapido las que estan ocupadas
        while(mem_address[pos].pid != -1)
            pos++;

        for(int i=pos; i<(pos + tamano); i++){
            if(mem_address[i].pid != -1){
                ocupado = true;
                pos = i + 1;
                break;
            }
        }

        if(!ocupado){
            for(int i=pos; i<(pos + tamano); i++){
                mem_address[i].pid = pthread_self();
                mem_address[i].estado = 0;
            }
            insertado = true;
            break;
        }
    }

    if(insertado){
        printf("(%ld: tamaño %d, tiempo %d)\n", pthread_self(), tamano, tiempo);

        // devuelve el semáforo
        sem_post(&mutex);
        
        sleep(tiempo);              // "Ejecuta"

        // pide el semáforo
        sem_wait(&mutex);

        // devuelve los recursos
        for(int i=pos; i<(pos + tamano); i++){
            mem_address[i].pid = -1;
            mem_address[i].estado = -1;
        }

    }else{
        printf("El hilo %ld de tamaño %d, murió porque no encontró amor\n",
                 pthread_self(), tamano);
    }

    // devuelve el semáforo
    sem_post(&mutex);

    return NULL;
}



int main()
{
    //crea la llave
    key_t llave_mem, llave_control;
    llave_mem = ftok(".",'x');
    llave_control = ftok(".",'a');

    // shmget me retorna el identificador de la memoria compartida, si existe
    int mem_id = shmget(llave_mem, 0, 0666);
    int control_id = shmget(llave_control, 0, 0666);

    if(mem_id == -1 || control_id == -1){
        printf("No hay acceso a la memoria compartida\n");
    }else{
        // shmat se pega a la memoria compartida
        mem_address = (Linea*) shmat(mem_id, (void*) 0, 0);
        control_address = (int*) shmat(control_id, (void*) 0, 0);

        if(mem_address == (void*)-1 || control_address == (void*)-1){
            printf("No se puede apuntar a la memoria compartida\n");
        }else{
            int cant_lineas = control_address[0];

            int algoritmo = 0;
            while(algoritmo < 1 || algoritmo > 3){
                printf("Algoritmos de asignación de memoria\n");
                printf("  1) First-Fit\n");
                printf("  2) Best-Fit\n");
                printf("  3) Worst-Fit\n");
                printf("______________________________________\n");
                printf("Seleccione el algoritmo a utilizar: ");
                scanf("%d", &algoritmo);

                if(algoritmo < 1 || algoritmo > 3){
                    printf("Por favor, ingrese una de las opciones.\n");
                }
                printf("\n");
            }

            // inicializa el semáforo para hilos
            sem_init(&mutex, 0, 1);

            // Así debería comportarse, maomeno
            while(control_address[1] == 1){         //mientras la memoria esté viva
                pthread_t proceso;
                struct Proceso info_proceso;

                info_proceso.tamano = getRandom(1, 10);
                info_proceso.tiempo = getRandom(20, 60);

                switch(algoritmo){
                    case 1:
                        pthread_create(&proceso, NULL, first_fit, (void *)&info_proceso);
                        break;
                    case 2:
                        printf("Aplicando Best-Fit...\n");
                        break;
                    case 3:
                        printf("Aplicando Worst-Fit...\n");
                        break;
                }


                int espera = getRandom(10, 20);
                printf("Nuevo hilo en %d segundos\n", espera);
                sleep(espera);
            }


            // destruye el semáforo
            sem_destroy(&mutex);

            //se despega de la memoria compartida
            shmdt(mem_address);
            shmdt(control_address);
        }
    }


    return 0;
}
