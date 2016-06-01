/*
 * planificador.h
 *
 *  Created on: 20/5/2016
 *      Author: utnso
 */

#ifndef PLANIFICADOR_H_
#define PLANIFICADOR_H_

#include <commons/collections/queue.h>
#include "pcb.h"
#include <stdlib.h>
#include <unistd.h>

t_list *lista_master_procesos;
t_queue *cola_listos;
t_queue *cola_bloqueados;

fd_set conjunto_cpus_libres;
fd_set conjunto_procesos_listos;
fd_set conjunto_procesos_bloqueados;
fd_set conjunto_procesos_ejecutando;
fd_set conjunto_procesos_salida;

typedef struct par_PCBs
{
	t_PCB viejo;
	t_PCB nuevo;
}par_PCBs;

#define STRUCT_PCB 300 			// Utilizar la funcion: mensaje_to_pcb() y pcb_to_mensaje() segun corresponda
#define FIN_QUANTUM 301
#define FIN_PROGRAMA 302
#define EJECUTAR 303
#define QUANTUM 304

int max_cpu, max_proceso;
t_PCB PCB_actualizado;

void roundRobin(const unsigned short int quantum, t_queue *listos, t_queue *bloqueados, t_queue *salida);

void mostrarEstados(void);
void esperaPorProcesos(t_queue*);
void ejecutar(t_PCB proceso, int quantum, int cpu);
void ponerListo(t_PCB *proceso);
void terminar(t_PCB *proceso);
void bloquear(t_PCB *proceso);
int estaLibre(int cpu);
void mostrarCola(const t_queue*);

int es_el_PCB_a_actualizar(t_PCB pcb);
t_mensaje quantum_to_mensaje(unsigned short int quantum);
//quizas haya que hacer que el tipo de dato de los parametros sea (void*)

#endif /* PLANIFICADOR_H_ */
