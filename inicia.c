#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>

//#include "linea.h"
#include "proceso.h"

//============================================================
//                  ALGORITMO DE BITÁCORA
//============================================================
// Crea o limmpia el archivo de bitácora
void crearBitacora(){
    //FILE *f = fopen ("/home/evelio/MemoriaCompartida-SO/bitacora.txt", "a");
    FILE *f = fopen ("bitacora.txt", "w");
    if(f == NULL){
        printf("Error al abrir el archivo\n");
        exit(1);
    }
    fclose(f);
}

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
    key_t llave_mem, llave_control, llave_estados;
    llave_mem = ftok(".",'x');
    llave_control = ftok(".",'a');
    llave_estados = ftok(".",'b');

    // shmget retorna el identificador de la memoria compartida
    int mem_id = shmget(llave_mem, cant_lineas*sizeof(Proceso), 0666|IPC_CREAT);
    int control_id = shmget(llave_control, 2*sizeof(int), 0666|IPC_CREAT);
    int estados_id = shmget(llave_estados, cant_lineas*sizeof(Proceso), 0666|IPC_CREAT);
    
    //Este código sirve como ejemplo para ver datos extra sobre la memoria compartida
    /*struct shmid_ds shmid_ds;
    printf ("The segment size = %ld\n", shmid_ds.shm_segsz);
    */

    if(mem_id == -1 || control_id == -1 || estados_id == -1){
        printf("No se pudo crear la memoria compartida\n");
    }else{
        // shmat se pega a la memoria compartida
        Proceso * mem_address = (Proceso *) shmat(mem_id,(void*)0,0);
        int * control_address = (int *) shmat(control_id,(void*)0,0);

        if(mem_address == (void *)-1 || control_address == (void *)-1){
            printf("No se puede apuntar a la memoria compartida\n");
        }else{
            control_address[0] = cant_lineas;   //cantidad de líneas de la memoria compartida
            control_address[1] = 1;             //la memoria compartida está viva (existe)
            control_address[2] = 0;             //cantidad de hilos vivos (para mostrar los estados)

            for(int i=0; i<cant_lineas; i++){
                mem_address[i].pid = -1;
                //mem_address[i].estado = -1;
            }

            crearBitacora();

            printf("¡Memoria compartida creada con éxito!\n");
            

            //se despega de la memoria compartida
            shmdt(mem_address);
            shmdt(control_address);
        }
    }


    return 0;
}
