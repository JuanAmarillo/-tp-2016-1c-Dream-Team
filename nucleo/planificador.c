#include "planificador.h"


void roundRobin(const unsigned short int quantum, t_queue *listos, t_queue *bloqueados, t_queue *salida)
{
	t_PCB *proceso;
	int cpu_explorer;

	//espera activa a que haya un primer proceso listo
	while(queue_is_empty(listos));

	while(1)
	{
		//Espera activa por una CPU

		for(cpu_explorer = 3; cpu_explorer <= max_cpu; ++cpu_explorer)
		{
			if(estaLibre(cpu_explorer))
			{
				//Extraer proceso de la cola de listos
				proceso = queue_pop(cola_listos);
				//Colocar proceso en la lista Ejecutando
				FD_SET(proceso->pid, &conjunto_procesos_ejecutando);
				//Establecer que la cpu ya no está libre
				FD_CLR(cpu_explorer, &conjunto_cpus_libres);
				//Ejecutar proceso
				ejecutar(*proceso, quantum, cpu_explorer);
			}
		}
	}
}

void ejecutar(t_PCB proceso, int quantum, int cpu)
{
	t_mensaje mensaje_PCB = pcb_to_mensaje(proceso,EJECUTAR);
	t_mensaje mensaje_quantum = quantum_to_mensaje(quantum);
	enviarMensaje(cpu, mensaje_PCB);
	enviarMensaje(cpu, mensaje_quantum);
}

t_mensaje quantum_to_mensaje(int quantum)
{
	t_mensajeHead head_quantum = {QUANTUM, 0, sizeof(int)};
	int *q = (int*) malloc(sizeof(int));
	t_mensaje mensaje_quantum = {head_quantum, NULL, (char*) q};
	return mensaje_quantum;
}

void mostrarEstados(void)
{
	int explorer_procesos;
	while(1)
	{
		system("clear");
		for(explorer_procesos = 0; explorer_procesos <= max_proceso; ++explorer_procesos)
		{
			printf("Proceso %d: ", explorer_procesos);
			if(FD_ISSET(explorer_procesos, &conjunto_procesos_listos)) printf("Listo\n");
			if(FD_ISSET(explorer_procesos, &conjunto_procesos_ejecutando)) printf("Ejecutando\n");
			if(FD_ISSET(explorer_procesos, &conjunto_procesos_bloqueados)) printf("Bloqueado\n");
			if(FD_ISSET(explorer_procesos, &conjunto_procesos_salida)) printf("Terminado\n");
		}
	}
}


int estaLibre(int cpu)
{
	return (FD_ISSET(cpu, &conjunto_cpus_libres));
}


void ponerListo(t_PCB *proceso)
{
	queue_push(cola_listos, proceso);
	FD_SET(proceso->pid, &conjunto_procesos_listos);
}

void terminar(t_PCB *proceso)
{
	FD_CLR(proceso->pid, &conjunto_procesos_ejecutando);
	FD_SET(proceso->pid, &conjunto_procesos_salida);
	free(proceso);
}

void bloquear(t_PCB *proceso)
{
	queue_push(cola_bloqueados, proceso);
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
