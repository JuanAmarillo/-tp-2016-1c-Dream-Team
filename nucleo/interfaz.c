#include "interfaz.h"

void interfaz(void)
{
	do
	{
		system("clear");

		imprimirListos();

		imprimirBloqueados();

		printf("____________________________________________\n");

		imprimirMaster();

		sleep(1);
	}while(1);
}

void imprimirListos(void)
{
	printf("____________________________________________\n");
	printf("Cola de Listos: ");
	imprimirListaProcesos(cola_listos->elements);
	if(!queue_is_empty(cola_listos))
	{
		system("tput setaf 3");
		printf("                 1°\n");
		system("tput setaf 9");
	}
}

void imprimirBloqueados(void)
{
	printf("____________________________________________\n");
	printf("Bloqueados: ");
	imprimirListaProcesos(cola_bloqueados->elements);

}

void imprimirListaProcesos(const t_list *lista)
{
	if(!lista->head)
	{
		printf("[]\n");
		return;
	}
	t_link_element *aux;
	putchar('[');
	for(aux = lista->head; aux->next; aux = aux->next)
	{
		printf("%d,", ((t_PCB*)(aux->data))->pid);
	}
	printf("%d]\n", ((t_PCB*)(aux->data))->pid);
}

void* master_imprimirProceso(void *pcb)
{
	t_PCB *aux_pcb = (t_PCB*) pcb;
	int pid = aux_pcb->pid;
	char estado[10+1];
	estado_to_string(aux_pcb->estado, estado);
	estado[10] = '\0';

	if(aux_pcb->estado == 1) system("tput setaf 4");
	if(aux_pcb->estado == 2) system("tput setaf 2");
	if(aux_pcb->estado == 3) system("tput setaf 3");
	if(aux_pcb->estado == 4)
	{
		if(FD_ISSET(aux_pcb->pid, &conjunto_procesos_abortados))
			strcpy(estado, "Abortado");
			system("tput setaf 1");
	}
	printf("%04d  |  ", pid);
	printf("%s\n", estado);
	return NULL;
}

void imprimirMaster(void)
{
	printf("___________________\n");
	printf("PID   |  Estado\n");
	printf("-------------------\n");
	list_map(lista_master_procesos, master_imprimirProceso);
	system("tput setaf 9");
}
