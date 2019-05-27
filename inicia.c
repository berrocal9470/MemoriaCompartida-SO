#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#include "linea.h"

int main()
{
    int cant_lineas = 0;
    while(cant_lineas <= 0){
        printf("Ingrese el número de líneas: ");
        scanf("%d", &cant_lineas);

        if(cant_lineas <= 0){
            printf("Por favor, ingrese un número positivo.\n\n");
        }
    }

    //crea las llaves para las memorias compartidas
    key_t llave_mem, llave_control;
    llave_mem = ftok(".",'x');
    llave_control = ftok(".",'a');

    // shmget retorna el identificador de la memoria compartida
    int mem_id = shmget(llave_mem, cant_lineas*sizeof(Linea), 0666|IPC_CREAT);
    int control_id = shmget(llave_control, sizeof(int), 0666|IPC_CREAT);

    
    //Este código sirve como ejemplo para ver datos extra sobre la memoria compartida
    /*struct shmid_ds shmid_ds;
    printf ("The segment size = %ld\n", shmid_ds.shm_segsz);
    */

    if(mem_id == -1 || control_id == -1){
        printf("No se pudo crear la memoria compartida\n");
    }else{
        // shmat se pega a la memoria compartida
        Linea *mem_address = (Linea *) shmat(mem_id,(void*)0,0);
        int *control_address = (int *) shmat(control_id,(void*)0,0);

        if(mem_address == (void *)-1 || control_address == (void *)-1){
            printf("No se puede apuntar a la memoria compartida\n");
        }else{
            control_address[0] = cant_lineas;   //cantidad de líneas de la memoria compartida
            control_address[1] = 1;             //la memoria compartida está viva (existe)

            for(int i=0; i<cant_lineas; i++){
                mem_address[i].pid = -1;
                mem_address[i].estado = -1;
            }

            printf("¡Memoria compartida creada con éxito!\n");
            

            //se despega de la memoria compartida
            shmdt(mem_address);
            shmdt(control_address);
        }
    }


    return 0;
}
