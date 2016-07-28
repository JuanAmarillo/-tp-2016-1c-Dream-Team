/*
 * E-S.h
 *
 *  Created on: 21/7/2016
 *      Author: utnso
 */

#ifndef NUCLEO_13_E_S_H_
#define NUCLEO_13_E_S_H_

#include "pcb.h"
#include "conjuntos_procesos.h"
#include "archivoLog.h"
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

void* llamar_planificarDispositivo(void *data);

void ponerListo(t_PCB *proceso);

#endif /* NUCLEO_13_E_S_H_ */
