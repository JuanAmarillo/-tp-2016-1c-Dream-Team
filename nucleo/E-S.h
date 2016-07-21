/*
 * E-S.h
 *
 *  Created on: 21/7/2016
 *      Author: utnso
 */

#ifndef E_S_H_
#define E_S_H_

#include "pcb.h"
#include <commons/collections/queue.h>
#include <unistd.h>

typedef struct
{
	t_PCB *proceso;
	unsigned int cantOp;
}t_parProcesoCantOp;

typedef struct
{
	char *nombre;
	unsigned int io_sleep;
	t_queue *cola;
	t_parProcesoCantOp *atendiendo;
} t_dispositivo;

#endif /* E_S_H_ */
