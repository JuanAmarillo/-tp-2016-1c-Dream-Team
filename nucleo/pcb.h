#ifndef NUCLEO_5_PCB_H_
#define NUCLEO_5_PCB_H_

#include <string.h>
#include <commons/collections/list.h>
#include "metadata_program.h"
#include "protocolo_mensaje.h"

/*
 * Estructuras de datos
 */
typedef struct {
	unsigned numeroPagina;
	unsigned offset;
	unsigned size;
} t_posicionDeMemoria;

typedef struct {
	char identificador;
	t_posicionDeMemoria posicionMemoria;
} t_variable;

typedef struct {
	t_list *args;
	t_list *vars;
	unsigned retPos;
	t_posicionDeMemoria retVar;
} t_indiceStack;

typedef struct {
	unsigned offset_inicio;
	unsigned offset_fin;
} t_indiceCodigo;

typedef struct {
	unsigned pid;
	unsigned pc;
	unsigned sp;
	unsigned cantidadPaginas;
	unsigned estado;
	unsigned tam_indiceEtiquetas;
	char *indiceEtiquetas;
	unsigned total_instrucciones;
	t_indiceCodigo *indiceCodigo;
	t_list *indiceStack;
} t_PCB;

typedef struct {
	unsigned cantidad_args;
	unsigned cantidad_vars;
} t_mensajeHeadStack;

/*
 * Funciones / Procedimientos
 */

//static t_indiceStack *stack_create(unsigned, unsigned, unsigned, unsigned);
//static t_posicionDeMemoria *args_create(unsigned, unsigned, unsigned);
//static t_variable *vars_create(char, unsigned, unsigned, unsigned);
t_mensaje pcb_to_mensaje(t_PCB, unsigned);
t_mensajeHeadStack desempaquetar_headStack(const void *);
t_PCB mensaje_to_pcb(t_mensaje);
//static void args_destroy(t_posicionDeMemoria *);
//static void vars_destroy(t_variable *);
//static void stack_destroy(t_indiceStack *);
t_PCB crearPCB(t_mensaje programa, unsigned int pid, unsigned int tamPag);
void freePCB(t_PCB *);
void testCrearPCB();

#endif /* NUCLEO_5_PCB_H_ */
