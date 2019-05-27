#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include <pthread.h>
#include <stdbool.h>

#include "linea.h"
#include "proceso.h"


// Variables globales
Linea * mem_address;
int * control_address; 

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

    /*printf("Tamaño: %d\n", actual->tamano);
    printf("Tiempo: %d\n", actual->tiempo);
    printf("-\n");*/

    int pos = 0;
    bool ocupado;
    bool insertado = false;
    while(pos < (control_address[0] + actual->tamano)){        //posicion menor que el tamaño de lineas
        ocupado = false;

        for(int i=pos; i<(pos + actual->tamano); i++){
            if(mem_address[i].pid != -1){
                ocupado = true;
                pos = i + 1;
                break;
            }
        }

        if(!ocupado){
            for(int i=pos; i<(pos + actual->tamano); i++){
                mem_address[i].pid = pthread_self();
                mem_address[i].estado = 0;
            }
            insertado = true;
            break;
        }
    }

    if(!insertado){
        printf("El hilo %ld de tamaño %d, murió porque no encontró amor\n",
                 pthread_self(), actual->tamano);
    }


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
        mem_address = (Linea*) shmat(mem_id,(void*)0,0);
        control_address = (int*) shmat(control_id,(void*)0,0);

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
                    printf("Por favor, ingrese una de las opciones.\n\n");
                }
            }


            switch(algoritmo){
                case 1:
                    printf("Usted eligió First-Fit\n\n");

                    // Así debería comportarse, maomeno
                    //while(control_address[1] == 1){         //mientras la memoria esté viva
                    for(int i=0; i<3; i++){ // <---- QUITAR ESTE FOR!!!!!!!!!!!!!!!
                        pthread_t proceso;
                        struct Proceso info_proceso;

                        info_proceso.tamano = getRandom(1, 10);
                        info_proceso.tiempo = getRandom(20, 60);

                        pthread_create(&proceso, NULL, first_fit, (void *)&info_proceso);

                        int espera = getRandom(3, 6);
                        printf("Durmiendo %d segundos\n", espera);
                        sleep(espera);
                    }

                    printf("-El código está pero no tiene semáforos.\n\n");

                    break;
                case 2:
                    printf("Usted eligió Best-Fit\n");
                    break;
                case 3:
                    printf("Usted eligió Worst-Fit\n");
                    break;
            }


            sleep(3); // QUITAR ESTO!!!!!!!!!!!!!!!!!!!


            /*while(control_address[1] == 1){         //mientras la memoria esté viva
                printf("soy un cacahuate\n");
                sleep(3);
            }*/

            //se despega de la memoria compartida
            shmdt(mem_address);
            shmdt(control_address);
        }
    }


    return 0;
}
