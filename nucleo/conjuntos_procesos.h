/*
 * conjuntos_procesos.h
 *
 *  Created on: 21/7/2016
 *      Author: utnso
 */

#ifndef NUCLEO_13_CONJUNTOS_PROCESOS_H_
#define NUCLEO_13_CONJUNTOS_PROCESOS_H_

fd_set conjunto_cpus_libres;

fd_set conjunto_procesos_listos;
pthread_mutex_t mutex_conjunto_procesos_listos;

fd_set conjunto_procesos_bloqueados;
fd_set conjunto_procesos_ejecutando;
fd_set conjunto_procesos_salida;
fd_set conjunto_procesos_abortados;

#endif /* NUCLEO_13_CONJUNTOS_PROCESOS_H_ */
