#ifndef PCB_H_
#define PCB_H_

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
	char* indiceEtiquetas;
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

static t_indiceStack *stack_create(unsigned, unsigned, unsigned, unsigned);
static t_posicionDeMemoria *args_create(unsigned, unsigned, unsigned);
static t_variable *vars_create(char, unsigned, unsigned, unsigned);
t_mensaje pcb_to_mensaje(t_PCB, unsigned);
t_mensajeHeadStack desempaquetar_headStack(const void *);
t_PCB mensaje_to_pcb(t_mensaje);
static void args_destroy(t_posicionDeMemoria *);
static void vars_destroy(t_variable *);
static void stack_destroy(t_indiceStack *);
void freePCB(t_PCB *);
void testCrearPCB();

#endif /* PCB_H_ */
