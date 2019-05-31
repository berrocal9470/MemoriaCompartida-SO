#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>

int main()
{
    //crea la llave
    key_t llave_mem, llave_control, llave_estados;
    llave_mem = ftok(".",'x');
    llave_control = ftok(".",'a');
    llave_estados = ftok(".",'b');

    // shmget me retorna el identificador de la memoria compartida
    int mem_id = shmget(llave_mem, 0, 0);
    int control_id = shmget(llave_control, 0, 0);
    int estados_id = shmget(llave_estados, 0, 0);

    if(mem_id == -1 || control_id == -1 || estados_id == -1){
        printf("No hay acceso a la memoria compartida\n");
    }else{
        printf("Eliminando...\n");

        // destruye la memoria compartida 
        shmctl(mem_id, IPC_RMID, NULL);
        shmctl(estados_id, IPC_RMID, NULL); 

        // Esto es para que el productor deje de producir y muera
        int *control_address = (int*) shmat(control_id,(void*)0,0);
        control_address[1] = 0;

        //sleep(4);   //espera al productor

        //destruye la memoria de control
        shmctl(control_id, IPC_RMID, NULL);

        printf("¡Memoria compartida eliminada con éxito!\n");
    }


    return 0;
}
