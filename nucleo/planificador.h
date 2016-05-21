/*
 * planificador.h
 *
 *  Created on: 20/5/2016
 *      Author: utnso
 */

#ifndef PLANIFICADOR_H_
#define PLANIFICADOR_H_

#include <commons/collections/queue.h>
#include <stdlib.h>

t_queue cola_listos;
t_queue cola_bloqueados;
t_queue cola_salida;

void roundRobin(const unsigned short int quantum, t_queue *listos, t_queue *bloqueados, t_queue *salida);

#endif /* PLANIFICADOR_H_ */
