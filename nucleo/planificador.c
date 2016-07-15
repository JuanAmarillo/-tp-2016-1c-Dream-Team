#include "planificador.h"

int imprimir_i = 1, vuelta = 0;
void roundRobin(const unsigned short int quantum, unsigned int quantumSleep, t_queue *listos, t_queue *bloqueados, t_queue *salida)
{
	t_PCB *proceso;
	int cpu_explorer;
	if(imprimir_i)
	{
		escribirLog("---------------------------\n");
		escribirLog("Ha iniciado el planificador\n");
		escribirLog("---------------------------\n");
		imprimir_i = 0;
	}

	while(1)
	{
		//espera activa a que haya un primer proceso listo
		esperaPorProcesos(listos);

		//Espera activa por una CPU
		for(cpu_explorer = 3; cpu_explorer <= max_cpu; ++cpu_explorer)
		{
			escribirLog("--------------------------------\n");
			escribirLog("Se analiza fd[%d] -->", cpu_explorer);
			if(estaLibre(cpu_explorer))
			{
				escribirLog("CPU Libre\n");
				escribirLog("--------------------------------\n");
				//Extraer proceso de la cola de listos
				proceso = queue_pop(listos);
				//Borrar del conjunto de procesos listos
				FD_CLR(proceso->pid, &conjunto_procesos_listos);
				//Colocar proceso en la lista Ejecutando
				FD_SET(proceso->pid, &conjunto_procesos_ejecutando);
				//Poner en estado ejecutando;
				proceso->estado = 1;
				//Establecer que la cpu ya no está libre
				FD_CLR(cpu_explorer, &conjunto_cpus_libres);
				//Ejecutar proceso
				ejecutar(*proceso, quantum, quantumSleep, cpu_explorer);

				break;
			}
			else
			{
				escribirLog("Nada\n");
				escribirLog("--------------------------------\n");
			}
		}
		escribirLog("vuelta: %d\n", ++vuelta);
		sleep(3);
	}
}

void esperaPorProcesos(t_queue* cola)
{
	//Si está vacía, informar
	if(queue_is_empty(cola)) escribirLog("La cola de Listos está vacía...\n");
	//Espera activa
	while(queue_is_empty(cola));
}

void ejecutar(t_PCB proceso, unsigned short int quantum, unsigned int qSleep, int cpu)
{
	t_mensaje mensaje_PCB = pcb_to_mensaje(proceso,STRUCT_PCB);

	t_mensaje mensaje_quantum = quantum_to_mensaje(quantum, qSleep);

	enviarMensaje(cpu, mensaje_PCB);
	enviarMensaje(cpu, mensaje_quantum);
	free(mensaje_quantum.parametros);
	actualizarMaster();
	escribirLog("-----------------------------------------\n");
	escribirLog("Se ejecutó el proceso %d en la cpu:fd[%d]\n", proceso.pid, cpu);
	escribirLog("Quantum:%d\n", quantum);
	escribirLog("Quantum Sleep:%d\n", qSleep);
	escribirLog("-----------------------------------------\n");
}

t_mensaje quantum_to_mensaje(unsigned short int quantum, unsigned int qSleep)
{
	t_mensajeHead head_quantum = {QUANTUM, 2, 0};
	t_mensaje mensaje_quantum;
	mensaje_quantum.head = head_quantum;
	mensaje_quantum.parametros = malloc(2*sizeof(unsigned));
	mensaje_quantum.parametros[0] = quantum;
	mensaje_quantum.parametros[1] = qSleep;
	mensaje_quantum.mensaje_extra = NULL;
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
			if(FD_ISSET(explorer_procesos, &conjunto_procesos_abortados)) printf("Abortado\n");
		}
	}
}


int estaLibre(int cpu)
{
	return (FD_ISSET(cpu, &conjunto_cpus_libres));
}

void* actualizar_si_corresponde(void *pcb)
{
	t_PCB *aux_pcb = (t_PCB*)pcb;
	if(FD_ISSET(aux_pcb->pid, &conjunto_procesos_listos)) aux_pcb->estado = 1;
	if(FD_ISSET(aux_pcb->pid, &conjunto_procesos_ejecutando)) aux_pcb->estado = 2;
	if(FD_ISSET(aux_pcb->pid, &conjunto_procesos_bloqueados)) aux_pcb->estado = 3;
	if(FD_ISSET(aux_pcb->pid, &conjunto_procesos_salida)) aux_pcb->estado = 4;
	if(FD_ISSET(aux_pcb->pid, &conjunto_procesos_abortados)) aux_pcb->estado = 4;//Sí, es el mismo estado que salida

	return NULL;
}

void actualizarMaster(void)
{
	list_map(lista_master_procesos, actualizar_si_corresponde);
}

void ponerListo(t_PCB *proceso)
{
	proceso->estado = 1;
	queue_push(cola_listos, proceso);
	FD_SET(proceso->pid, &conjunto_procesos_listos);
	actualizarMaster();
}

void terminar(t_PCB *proceso)
{
	FD_CLR(proceso->pid, &conjunto_procesos_ejecutando);
	FD_SET(proceso->pid, &conjunto_procesos_salida);
	proceso->estado = 4;
	actualizarMaster();
	escribirLog("Termino el proceso %d\n", proceso->pid);
	free(proceso);
}

void bloquear(t_PCB *proceso)
{
	FD_CLR(proceso->pid, &conjunto_procesos_ejecutando);
	FD_SET(proceso->pid, &conjunto_procesos_bloqueados);
	proceso->estado = 3;
	queue_push(cola_bloqueados, proceso);
	actualizarMaster();
}

void *imprimirPID(void *pcb)
{
	printf("%d\n", ((t_PCB*)pcb)->pid);
	return NULL;
}

void mostrarCola(const t_queue* cola)
{
	list_map(cola->elements, imprimirPID);
}
