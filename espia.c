#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#include "linea.h"

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
        Linea *mem_address = (Linea*) shmat(mem_id,(void*)0,0);
        int *control_address = (int*) shmat(control_id,(void*)0,0);

        if(mem_address == (void*)-1 || control_address == (void*)-1){
            printf("No se puede apuntar a la memoria compartida\n");
        }else{
            int cant_lineas = control_address[0];

            printf("N°       PID                 Estado\n");
            printf("----------------------------------------\n");

            //printf("Data written in memory: %s\n", str);
            for(int i=0; i<cant_lineas; i++){
                printf("%-9d", i);

                if(mem_address[i].pid == -1){
                    printf("%-20s", "-");
                }else{
                    printf("%-20ld", mem_address[i].pid);
                }

                switch(mem_address[i].estado %2){           //EL MOD ES SOLO DE EJEMPLO!!!!!!!!!
                    case -1:
                        printf("-\n");
                        break;
                    case 0:
                        printf("Ejecutando\n");
                        break;
                    case 1:
                        printf("Bloqueado\n");
                        break;
                }
                printf("----------------------------------------\n");
            }

            //se despega de la memoria compartida
            shmdt(mem_address);
            shmdt(control_address);
        }
    }


    return 0;
}
