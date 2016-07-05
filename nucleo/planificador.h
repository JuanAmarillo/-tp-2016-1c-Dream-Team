/*
 * planificador.h
 *
 *  Created on: 20/5/2016
 *      Author: utnso
 */

#ifndef NUCLEO_5_PLANIFICADOR_H_
#define NUCLEO_5_PLANIFICADOR_H_

#include <commons/collections/queue.h>
#include <stdlib.h>
#include <unistd.h>
#include "archivoLog.h"
#include "pcb.h"

t_list *lista_master_procesos;
t_queue *cola_listos;
t_queue *cola_bloqueados;

fd_set conjunto_cpus_libres;
fd_set conjunto_procesos_listos;
fd_set conjunto_procesos_bloqueados;
fd_set conjunto_procesos_ejecutando;
fd_set conjunto_procesos_salida;
fd_set conjunto_procesos_abortados;

typedef struct
{
	int pid;
	int fd_consola;
}t_parPidConsola;

t_list *lista_Pares;

//Con la CPU
#define STRUCT_PCB 300 			// Utilizar la funcion: mensaje_to_pcb() y pcb_to_mensaje() segun corresponda
#define FIN_QUANTUM 301
#define FIN_PROGRAMA 302
#define EJECUTAR 303
#define QUANTUM 304
#define BLOQUEADO 305

//Con la UMC
#define RETURN_TAM_PAGINA 705
#define ALMACENAR_OK 310
#define ALMACENAR_FAILED 320

#define INIT_PROG 300
#define FIN_PROG 305

//Con la Consola
#define IMPRIMIR_PROGRAMA 101
#define IMPRIMIR_TEXTO_PROGAMA 102

int max_cpu, max_proceso;
t_PCB PCB_actualizado;

void roundRobin(const unsigned short int quantum, unsigned int quantumSleep, t_queue *listos, t_queue *bloqueados, t_queue *salida);

void mostrarEstados(void);
void esperaPorProcesos(t_queue*);
void ejecutar(t_PCB proceso, unsigned short int quantum, unsigned int quantumSleep, int cpu);
void ponerListo(t_PCB *proceso);
void terminar(t_PCB *proceso);
void bloquear(t_PCB *proceso);
int estaLibre(int cpu);
void mostrarCola(const t_queue*);
void actualizarMaster(void);

int es_el_PCB_a_actualizar(t_PCB pcb);
t_mensaje quantum_to_mensaje(unsigned short int quantum, unsigned int quantumSleep);
//quizas haya que hacer que el tipo de dato de los parametros sea (void*)

#endif /* NUCLEO_5_PLANIFICADOR_H_ */
