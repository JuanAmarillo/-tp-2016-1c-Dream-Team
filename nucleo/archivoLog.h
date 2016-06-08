/*
 * archivoLog.h
 *
 *  Created on: 7/6/2016
 *      Author: utnso
 */

#ifndef ARCHIVOLOG_H_
#define ARCHIVOLOG_H_
#include <stdio.h>
#include <stdarg.h>
#include "pcb_5.h"
#include <commons/collections/queue.h>

FILE *archivoLog;

void crearLog(void);//Sirve para crear el archivo y en caso de que ya exista vaciarlo para eliminar el log anterior
int escribirLog(const char*, ...);//Es como printf pero escribe sobre el archivo de log en lugar de stdout
void mostrarColaPorLog(const t_queue* cola);//improme una cola de procesos por archivo de log

#endif /* ARCHIVOLOG_H_ */
