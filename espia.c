#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>

//#include "linea.h"
#include "proceso.h"

int main()
{
    //crea la llave
    key_t llave_mem, llave_control, llave_estados;
    llave_mem = ftok(".",'x');
    llave_control = ftok(".",'a');
    llave_estados = ftok(".",'b');

    // shmget me retorna el identificador de la memoria compartida, si existe
    int mem_id = shmget(llave_mem, 0, 0666);
    int control_id = shmget(llave_control, 0, 0666);
    int estados_id = shmget(llave_estados, 0, 0666);

    if(mem_id == -1 || control_id == -1 || estados_id == -1){
        printf("No hay acceso a la memoria compartida\n");
    }else{
        // shmat se pega a la memoria compartida
        Proceso * mem_address = (Proceso *) shmat(mem_id, (void*)0, 0);
        int * control_address = (int*) shmat(control_id, (void*)0, 0);
        Proceso * estados_address = (Proceso *) shmat(estados_id, (void*)0, 0);

        if(mem_address == (void*)-1 || control_address == (void*)-1 
            || estados_address == (void*)-1){
            printf("No se puede apuntar a la memoria compartida\n");
        }else{
            int cant_lineas = control_address[0];

            printf("======= Procesos vivos ========\n");
            printf("PID                 Estado\n");
            printf("-------------------------------\n");
            for(int i=0; i<control_address[2]; i++){
                printf("%-20ld", estados_address[i].pid);
                switch(estados_address[i].estado){
                    case Fuera:
                        printf("Fuera");
                        break;
                    case Bloqueado:
                        printf("Bloqueado");
                        break;
                    case Ejecutando:
                        printf("Ejecutando");
                        break;
                    case RegionCritica:
                        printf("En Región Crítica");
                        break;
                }
                printf("\n-------------------------------\n");
            }

            printf("\n====== Memoria Ready =======\n");
            printf("N°       PID\n");
            printf("----------------------------\n");

            //printf("Data written in memory: %s\n", str);
            for(int i=0; i<cant_lineas; i++){
                printf("%-9d", i);

                if(mem_address[i].pid == -1){
                    printf("%-20s", "-");
                }else{
                    printf("%-20ld", mem_address[i].pid);
                }

                printf("\n----------------------------\n");
            }

            //se despega de la memoria compartida
            shmdt(mem_address);
            shmdt(control_address);
            shmdt(estados_address);
        }
    }


    return 0;
}
