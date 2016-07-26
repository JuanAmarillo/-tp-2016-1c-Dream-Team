/*
 * semaforos.h
 *
 *  Created on: 26/7/2016
 *      Author: utnso
 */

#ifndef SEMAFOROS_H_
#define SEMAFOROS_H_

typedef struct
{
	char *id;
	int cuenta;
	t_queue *cola;
}t_semaforo;

t_semaforo *semaforos;
unsigned int cantidadSemaforos;

void sem_wait(t_PCB *proceso, t_semaforo *semaforo);
void sem_signal(t_PCB *proceso, t_semaforo *semaforo);

#endif /* SEMAFOROS_H_ */
