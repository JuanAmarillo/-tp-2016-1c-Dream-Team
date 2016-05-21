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

t_queue cola_listos;
t_queue cola_bloqueados;
t_queue cola_salida;

void roundRobin(const unsigned short int quantum, t_queue *listos, t_queue *bloqueados, t_queue *salida);

int ejecutar(t_PCB proceso, int cpu);
void terminar(t_PCB proceso);
void bloquear(t_PCB proceso);

#endif /* PLANIFICADOR_H_ */
