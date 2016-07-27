/*
 * semaforos.h
 *
 *  Created on: 26/7/2016
 *      Author: utnso
 */

#ifndef SEMAFOROS_H_
#define SEMAFOROS_H_

#include "pcb.h"
#include "archivoLog.h"
#include "planificador.h"
#include <commons/collections/queue.h>

typedef struct
{
	char *id;
	int cuenta;
	t_queue *cola;
}t_semaforo;

t_semaforo *semaforos;
unsigned int cantidadSemaforos;

int semaforo_wait(t_PCB *proceso, t_semaforo *semaforo);
void semaforo_signal(t_semaforo *semaforo);
t_semaforo* nombre_to_semaforo(const char *nombre);
int existeSemaforo(const char *nombre);

void bloquear(t_PCB *proceso, const char *dispositivo, unsigned int cantOp);
void ponerListo(t_PCB *proceso);
int eliminarProcesoSegunPID(t_list *lista, int pid);

#endif /* SEMAFOROS_H_ */
