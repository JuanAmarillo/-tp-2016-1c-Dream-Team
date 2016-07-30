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
#include "messageCode.h"
#include "E-S.h"
#include <pthread.h>
#include "conjuntos_procesos.h"

t_list *lista_master_procesos;
t_queue *cola_listos;
t_queue *cola_bloqueados;

t_dispositivo *vector_dispositivos;

t_queue *cola_cpus_disponibles;

typedef struct
{
	int pid;
	int fd_consola;
}t_parPidConsola;

typedef struct
{
	int pid;
	int fd_cpu;
}t_parPidCPU;

t_list *lista_Pares;//asocia cada pid con su consola

t_list *lista_CPUS_PIDS; // Asocia a cada cpu con el pid que esta ejecutando

int max_cpu, max_proceso;
t_PCB PCB_actualizado;

void roundRobin(const unsigned short int quantum, unsigned int quantumSleep, t_queue *listos, t_queue *bloqueados, t_queue *salida);

void mostrarEstados(void);
void esperaPorProcesos(t_queue*);
void esperaPorCPUs(void);
void ejecutar(t_PCB proceso, unsigned short int quantum, unsigned int quantumSleep, int cpu);
void ponerListo(t_PCB *proceso);
void terminar(t_PCB *proceso);
void bloquear(t_PCB *proceso, const char *dispositivo, unsigned int cantOp);
int estaLibre(int cpu);
void mostrarCola(const t_queue*);
void actualizarMaster(void);

void habilitarCPU(int);
void deshabilitarCPU(int);

int cantidadDispositivos(void);
void abortarProceso(int pid);

void asociarPidCPU(int pid, int cpu);
void desasociarPidCPU(int pid);
int CPU_to_Pid(int cpu);
int Pid_to_CPU(int pid);

int releerQuantum(void);
int releerQuantumSleep(void);

int es_el_PCB_a_actualizar(t_PCB pcb);
t_mensaje quantum_to_mensaje(unsigned short int quantum, unsigned int quantumSleep);

pthread_t* comenzar_Planificador_EntradaSalida(void);

#endif /* NUCLEO_5_PLANIFICADOR_H_ */
