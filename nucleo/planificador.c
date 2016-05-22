#include "planificador.h"

void roundRobin(const unsigned short int quantum, t_queue *listos, t_queue *bloqueados, t_queue *salida)
{
	t_PCB proceso;
	int explorer;
	while(1)
	{
		while(queue_is_empty(listos) && queue_is_empty(bloqueados) && queue_is_empty(salida));
		for(explorer = 3; explorer <= max_cpu; ++explorer)
		{
			if(estaLibre(explorer))
			{
				if(!queue_is_empty(listos))
				{
					memcpy(&proceso, queue_pop((void*) &listos), sizeof(t_PCB));
					FD_CLR(explorer, &conjunto_cpus_libres);
					ejecutar(proceso, explorer);
					continue;
				}
			}
		}

		if(!agotoQuantum(&PCB_actualizado) && !agotoRafaga(&PCB_actualizado))
			devolverPCB(PCB_actualizado);

		if(list_any_satisfy(&lista_ejecutando, agotoQuantum))
		{
			//liberar CPU asociada al proceso
			FD_SET(cpu_asociada_al_proceso(proceso), &conjunto_cpus_libres);
			//sacar de lista de Ejecutando;
			memcpy(&proceso, list_remove_by_condition(&lista_ejecutando, agotoQuantum), sizeof(t_PCB));
			//poner en cola de Listo
			ponerListo(&proceso);
			continue;
		}

		if(list_any_satisfy(&lista_ejecutando, agotoRafaga))
		{
			//sacar de lista de Ejecutando;
			memcpy(&proceso, list_remove_by_condition(&lista_ejecutando, agotoQuantum), sizeof(t_PCB));
			//poner en cola de Salida
			terminar(&proceso);
		}

	}
}

int estaLibre(int cpu)
{
	return (FD_ISSET(cpu, &conjunto_cpus_libres));
}

void ejecutar(t_PCB proceso, int cpu)
{
	t_mensaje mensaje = pcb_to_mensaje(proceso,EJECUTAR);
	enviarMensaje(cpu, mensaje);
}

void devolverPCB(t_PCB proceso)
{
	ejecutar(proceso, cpu_asociada_al_proceso(proceso));
}

void ponerListo(t_PCB *proceso)
{
	queue_push(&cola_listos, proceso);
}

void terminar(t_PCB *proceso)
{
	queue_push(&cola_salida, proceso);
}

void bloquear(t_PCB *proceso)
{
	queue_push(&cola_bloqueados, proceso);
}

void actualizar_listaEjecutando(void)
{
	list_map(&lista_ejecutando, actualizar_si_corresponde);
}

void* actualizar_si_corresponde(void *pcb)
{
	if(es_el_PCB_a_actualizar(*((t_PCB*)pcb)))
		memcpy(pcb, &PCB_actualizado, sizeof(t_PCB));
	return NULL;
}

int es_el_PCB_a_actualizar(t_PCB pcb)
{
	return ( pcb.pid == PCB_actualizado.pid );
}

bool agotoQuantum(void *proceso)
{
	//Aún sin definir
	return true;
}
bool agotoRafaga(void *proceso)
{
	//Aún sin definir
	return true;
}
int cpu_asociada_al_proceso(t_PCB proceso)
{
	//Aún sin definir
	return 1;
}
