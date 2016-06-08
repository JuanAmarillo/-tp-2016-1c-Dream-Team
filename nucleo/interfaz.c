#include "interfaz.h"

void interfaz(void)
{
	do
	{
		system("clear");
		printf("PID     Estado\n");
		imprimirMaster();
		sleep(1);
	}while(1);
}

void* imprimirProceso(void *pcb)
{
	t_PCB *aux_pcb = (t_PCB*) pcb;
	int pid = aux_pcb->pid;
	char estado[10+1];
	estado_to_string(aux_pcb->estado, estado);
	estado[10] = '\0';

	printf("%04d    ", pid);
	if(aux_pcb->estado == 1) system("tput setaf 4");
	if(aux_pcb->estado == 2) system("tput setaf 2");
	if(aux_pcb->estado == 3) system("tput setaf 1");
	printf("%s\n", estado);
	system("tput setaf 9");
	return NULL;
}

void imprimirMaster(void)
{
	list_map(lista_master_procesos, imprimirProceso);
}
