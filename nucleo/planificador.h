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

t_list lista_master_procesos;
t_queue cola_listos;
t_queue cola_bloqueados;
t_queue cola_salida;
t_list lista_ejecutando;

fd_set conjunto_master_cpus;
fd_set conjunto_cpus_libres;

#define EJECUTAR 204//Ejemplo

int max_cpu;
t_PCB PCB_actualizado;

void roundRobin(const unsigned short int quantum, t_queue *listos, t_queue *bloqueados, t_queue *salida);

void ejecutar(t_PCB proceso, int cpu);
void ponerListo(t_PCB *proceso);
void terminar(t_PCB *proceso);
void bloquear(t_PCB *proceso);
int estaLibre(int cpu);

void actualizar_listaEjecutando(void);
bool agotoQuantum(void*);
bool agotoRafaga(void*);
void* actualizarPCB(t_PCB*);
void* actualizar_si_corresponde(void *pcb);
int cpu_asociada_al_proceso(t_PCB proceso);
void devolverPCB(t_PCB);
int es_el_PCB_a_actualizar(t_PCB pcb);

int ejecutar(t_PCB proceso, int cpu);
void terminar(t_PCB proceso);
void bloquear(t_PCB proceso);

#endif /* PLANIFICADOR_H_ */
