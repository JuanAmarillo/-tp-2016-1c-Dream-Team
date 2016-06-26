#include "pcb.h"

/*
 * stack_create();
 * Parametros:
 * 		-> retPos :: Direccion de retorno (Posición del índice de código donde se debe retornar al finalizar la ejecución de la función)
 * 		-> numeroPagina, offset, size :: Posición de la variable de retorno  (Posición de memoria donde se debe almacenar el resultado de la función provisto por la sentencia RETURN)
 * Descripcion: Crea un nuevo nodo con la estructura t_indiceStack
 * Return: puntero a nodo
 */
static t_indiceStack *stack_create(unsigned retPos, unsigned numeroPagina, unsigned offset, unsigned size){
	t_indiceStack *new = malloc(sizeof(t_indiceStack));
	new->args = list_create();
	new->vars = list_create();
	new->retPos = retPos;
	new->retVar.numeroPagina = numeroPagina;
	new->retVar.offset = offset;
	new->retVar.size = size;
	return new;
}

/*
 * args_create();
 * Parametros:
 * 		-> numeroPagina, offset, size :: Posiciones de memoria donde se almacenan la copia del argumento de la función
 * Descripcion: Crea un nuevo nodo con la estructura t_posicionDeMemoria
 * Return: puntero a nodo
 */
static t_posicionDeMemoria *args_create(unsigned numeroPagina, unsigned offset, unsigned size){
	t_posicionDeMemoria *new = malloc(sizeof(t_posicionDeMemoria));
	new->numeroPagina = numeroPagina;
	new->offset = offset;
	new->size = size;
	return new;
}

/*
 * vars_create();
 * Parametros:
 * 		-> identificador :: Nombre de la variable
 * 		-> numeroPagina, offset, size :: Posicion de memoria donde se almacenan la variable local de la función
 * Descripcion: Crea un nuevo nodo con la estructura t_posicionDeMemoria
 * Return: puntero a nodo
 */
static t_variable *vars_create(char identificador, unsigned numeroPagina, unsigned offset, unsigned size){
	t_variable *new = malloc(sizeof(t_variable));
	new->identificador = identificador;
	new->posicionMemoria.numeroPagina = numeroPagina;
	new->posicionMemoria.offset = offset;
	new->posicionMemoria.size = size;
	return new;
}

/*
 * pcb_to_mensaje();
 * Parametros:
 * 		-> pcb :: un PCB
 * 		-> codigo :: Codigo de operacion del mensaje
 * Descripcion: Convierte un PCB en estructura t_mensaje
 * Return: t_mensaje
 */
t_mensaje pcb_to_mensaje(t_PCB pcb, unsigned codigo) {

	// Variables usadas
	int cantidad_indiceCodigo = pcb.total_instrucciones;
	unsigned tam_indiceEtiquetas = pcb.tam_indiceEtiquetas;
	int cantidad_indiceStack = list_size(pcb.indiceStack);
	int cantidadTotal_args = 0;
	int cantidadTotal_vars = 0;
	unsigned tam_extra = 0;
	int tam_headStack = sizeof(unsigned) * 2;
	unsigned desplazamiento = 0;
	unsigned i_parametro;
	int cantidad_args = 0;
	int cantidad_vars = 0;
	t_mensaje mensaje;


	// Seteo cantidadTotal_args y cantidadTotal_vars
	if(cantidad_indiceStack != 0){
		void _list_elements(t_indiceStack *tmp) {
			cantidadTotal_args += list_size(tmp->args);
			cantidadTotal_vars += list_size(tmp->vars);
		}
		list_iterate(pcb.indiceStack, (void*) _list_elements);
	}

	// El tamaño de Payload + Head Internos
	tam_extra = tam_indiceEtiquetas + sizeof(t_indiceCodigo) * cantidad_indiceCodigo + (tam_headStack + sizeof(unsigned) + sizeof(t_posicionDeMemoria)) * cantidad_indiceStack + sizeof(t_posicionDeMemoria) * cantidadTotal_args + sizeof(t_variable) * cantidadTotal_vars;

	// Creo un bloque en memoria con la tamaño del Head + Payload
	char *mensaje_empaquetado = malloc(tam_extra);

	// Copio los indiceCodigo
	for (i_parametro = 0; i_parametro < cantidad_indiceCodigo; i_parametro++){
		memcpy(mensaje_empaquetado + desplazamiento, &pcb.indiceCodigo[i_parametro], sizeof(t_indiceCodigo));
		desplazamiento += sizeof(t_indiceCodigo);
	}

	// Copio indiceEtiquetas
	memcpy(mensaje_empaquetado + desplazamiento, pcb.indiceEtiquetas, tam_indiceEtiquetas);
	desplazamiento += tam_indiceEtiquetas;

	// Copio la lista de stack
	if(cantidad_indiceStack != 0){
		void _list_elements2(t_indiceStack *tmp2) {
			cantidad_args = list_size(tmp2->args);
			cantidad_vars = list_size(tmp2->vars);
			//
			memcpy(mensaje_empaquetado + desplazamiento, &cantidad_args, sizeof(int));
			desplazamiento += sizeof(int);
			memcpy(mensaje_empaquetado + desplazamiento, &cantidad_vars, sizeof(int));
			desplazamiento += sizeof(int);

			// Copio los args
			if(cantidad_args != 0){
				void _list_elements3(t_posicionDeMemoria *tmp3) {
					memcpy(mensaje_empaquetado + desplazamiento, tmp3, sizeof(t_posicionDeMemoria));
					desplazamiento += sizeof(t_posicionDeMemoria);
				}
				list_iterate(tmp2->args, (void*) _list_elements3);
			}
			// Copio los vars
			if(cantidad_vars != 0){
				void _list_elements4(t_variable *tmp4) {
					memcpy(mensaje_empaquetado + desplazamiento, tmp4, sizeof(t_variable));
					desplazamiento += sizeof(t_variable);
				}
				list_iterate(tmp2->vars, (void*) _list_elements4);
			}
			//
			memcpy(mensaje_empaquetado + desplazamiento, &(tmp2->retPos), sizeof(unsigned));
			desplazamiento += sizeof(unsigned);
			memcpy(mensaje_empaquetado + desplazamiento, &(tmp2->retVar), sizeof(t_posicionDeMemoria));
			desplazamiento += sizeof(t_posicionDeMemoria);
			// Reset
			cantidad_args = 0;
			cantidad_vars = 0;
		}
		list_iterate(pcb.indiceStack, (void*) _list_elements2);
	}

	// Devuelvo
	// Creo mensaje
	unsigned cantidad_parametros = 8;
	unsigned *parametros = malloc(sizeof(unsigned) * cantidad_parametros);
	parametros[0] = cantidad_indiceCodigo;
	parametros[1] = cantidad_indiceStack;
	parametros[2] = pcb.pid;
	parametros[3] = pcb.pc;
	parametros[4] = pcb.cantidadPaginas;
	parametros[5] = pcb.estado;
	parametros[6] = pcb.sp;
	parametros[7] = pcb.tam_indiceEtiquetas;

	mensaje.head.codigo = codigo;
	mensaje.head.cantidad_parametros = cantidad_parametros;
	mensaje.head.tam_extra = tam_extra;
	mensaje.mensaje_extra = mensaje_empaquetado;
	mensaje.parametros = parametros;
	//
	return mensaje;
}


/*
 * t_mensajeHeadStack();
 * Parametros:
 * 		-> buffer :: Un buffer
 * Descripcion: Dado un buffer desempaqueta el Head del Stack
 * Return: t_mensajeHeadStack
 */
t_mensajeHeadStack desempaquetar_headStack(const void *buffer) {

	// Declaro variables usadas
	unsigned desplazamiento = 0;
	t_mensajeHeadStack mensaje_HeadStack;

	// Desempaqueto el head
	memcpy(&mensaje_HeadStack.cantidad_args, buffer, sizeof(int));
	desplazamiento += sizeof(int);
	memcpy(&mensaje_HeadStack.cantidad_vars, buffer + desplazamiento, sizeof(int));
	desplazamiento += sizeof(int);

	return mensaje_HeadStack;
}

/*
 * mensaje_to_pcb();
 * Parametros:
 * 		-> mensaje :: Un t_mensaje
 * Descripcion: Dado un mensaje, lo convierte en t_PCB
 * Return: t_PCB
 */
t_PCB mensaje_to_pcb(t_mensaje mensaje) {

	// Declaro variables usadas
	unsigned i_parametro;
	unsigned i_stack;
	unsigned i_args;
	unsigned i_vars;
	unsigned desplazamiento = 0;
	t_mensajeHeadStack headStack;
	t_indiceStack *aux_stack;
	t_posicionDeMemoria aux_args;
	t_variable aux_vars;
	unsigned cantidad_indiceCodigo = mensaje.parametros[0];
	unsigned cantidad_indiceStack = mensaje.parametros[1];
	unsigned tam_indiceEtiquetas = mensaje.parametros[7];


	// Creo memoria para el PCB
	t_PCB pcb;

	pcb.total_instrucciones = cantidad_indiceCodigo;
	pcb.pid = mensaje.parametros[2];
	pcb.pc = mensaje.parametros[3];
	pcb.cantidadPaginas = mensaje.parametros[4];
	pcb.estado = mensaje.parametros[5];
	pcb.sp = mensaje.parametros[6];
	pcb.tam_indiceEtiquetas = tam_indiceEtiquetas;

	pcb.indiceCodigo = malloc(sizeof(t_indiceCodigo) * cantidad_indiceCodigo);
	pcb.indiceEtiquetas = malloc(tam_indiceEtiquetas);

	char *buffer = mensaje.mensaje_extra;

	// Desempaqueto el indiceCodigo
	for (i_parametro = 0; i_parametro < cantidad_indiceCodigo; i_parametro++){
		memcpy(&pcb.indiceCodigo[i_parametro], buffer + desplazamiento, sizeof(t_indiceCodigo));
		desplazamiento += sizeof(t_indiceCodigo);
	}

	// Paso indiceEtiquetas
	memcpy(pcb.indiceEtiquetas, buffer + desplazamiento, tam_indiceEtiquetas);
	desplazamiento += tam_indiceEtiquetas;

	pcb.indiceStack = list_create();
	// Desempaqueto el indiceStack
	for (i_stack = 0; i_stack < cantidad_indiceStack; i_stack++){
		// Creo Stack
		list_add(pcb.indiceStack, stack_create(0, 0, 0, 0));
		aux_stack = list_get(pcb.indiceStack, i_stack);
		//
		headStack = desempaquetar_headStack(buffer + desplazamiento);
		desplazamiento += sizeof(t_mensajeHeadStack);
		for (i_args = 0; i_args < headStack.cantidad_args; i_args++){
			memcpy(&aux_args, buffer + desplazamiento, sizeof(t_posicionDeMemoria));
			desplazamiento += sizeof(t_posicionDeMemoria);
			list_add(aux_stack->args, args_create(aux_args.numeroPagina, aux_args.offset, aux_args.size));
		}
		for (i_vars = 0; i_vars < headStack.cantidad_vars; i_vars++){
			memcpy(&aux_vars, buffer + desplazamiento, sizeof(t_variable));
			desplazamiento += sizeof(t_variable);
			list_add(aux_stack->vars, vars_create(aux_vars.identificador, aux_vars.posicionMemoria.numeroPagina, aux_vars.posicionMemoria.offset, aux_vars.posicionMemoria.size));
		}
		memcpy(&(aux_stack->retPos), buffer + desplazamiento, sizeof(unsigned));
		desplazamiento += sizeof(unsigned);
		memcpy(&(aux_stack->retVar), buffer + desplazamiento, sizeof(t_posicionDeMemoria));
		desplazamiento += sizeof(t_posicionDeMemoria);
	}

	return pcb;
}

/*
 * Funciones para destruir las listas
 */
static void args_destroy(t_posicionDeMemoria *self) {
    free(self);
}

static void vars_destroy(t_variable *self) {
    free(self);
}

static void stack_destroy(t_indiceStack *self) {
    free(self->args);
    free(self->vars);
    free(self);
}

/*
 * freePCB();
 * Parametros:
 * 		-> pcb :: Direccion de memoria de un PCB
 * Descripcion: Dado un PCB, libera toda su memoria
 */
void freePCB(t_PCB *pcb){
	int cantidad_indiceStack = list_size(pcb->indiceStack);
	if(cantidad_indiceStack != 0){
		void _list_elements2(t_indiceStack *tmp2) {
			// Destruyo los args
			list_destroy_and_destroy_elements(tmp2->args, (void*) args_destroy);
			// Destruyo los vars
			list_destroy_and_destroy_elements(tmp2->vars, (void*) vars_destroy);
		}
		list_iterate(pcb->indiceStack, (void*) _list_elements2);
	}
	// Destruyo los Stack
	list_destroy_and_destroy_elements(pcb->indiceStack, (void*) stack_destroy);
	//
	free(pcb->indiceEtiquetas);
}


t_PCB crearPCB(t_mensaje programa, unsigned int pid, unsigned int tamPag)
{
	t_PCB pcb;
	unsigned int tamCod = programa.head.tam_extra;
	char *codigo = malloc(tamCod);
	strcpy(codigo, programa.mensaje_extra);

	t_metadata_program *metadata = metadata_desde_literal(codigo);

	pcb.pid = pid;
	pcb.pc = metadata->instruccion_inicio;
	pcb.sp = 0;//Valor Provisorio necesita informacion de la UMC
	pcb.cantidadPaginas = (tamCod/tamPag) + 1;
	pcb.estado = 0;//Nuevo
	pcb.indiceEtiquetas = metadata->etiquetas;
	pcb.tam_indiceEtiquetas = metadata->etiquetas_size;
	pcb.total_instrucciones = metadata->instrucciones_size;

	pcb.indiceCodigo = (t_indiceCodigo*) metadata->instrucciones_serializado;
	pcb.indiceStack = list_create();//Valor Provisorio necesita informacion de la UMC
	return pcb;
}


/*
 * testCrearPCB();
 */
void testCrearPCB(){
	t_PCB pcb;
	t_indiceStack *aux_stack;

	t_indiceCodigo parametros[2];
	parametros[0].offset_fin = 53;
	parametros[0].offset_inicio = 54;
	parametros[1].offset_fin = 55;
	parametros[1].offset_inicio = 56;

	pcb.pid = 1;
	pcb.pc = 22;
	pcb.cantidadPaginas = 30;
	pcb.estado = 13;
	pcb.indiceCodigo = parametros;
	pcb.indiceStack = list_create();
	pcb.sp = 30;
	pcb.total_instrucciones = 2;
	pcb.tam_indiceEtiquetas = 5;

	char etiquetas[5];
	etiquetas[0] = 'h';
	etiquetas[1] = 'o';
	etiquetas[2] = 'l';
	etiquetas[3] = 'a';
	etiquetas[4] = '\0';

	pcb.indiceEtiquetas = etiquetas;

	// Creo un nodo STACK
	list_add(pcb.indiceStack, stack_create(40, 41, 42, 43));

	// Le agrego contenido a las listas (args y vars) del nodo Stack
	aux_stack = list_get(pcb.indiceStack, 0);
	list_add(aux_stack->args, args_create(60, 61, 62));
	list_add(aux_stack->args, args_create(63, 64, 65));
	list_add(aux_stack->vars, vars_create('a', 66, 67, 68));
	list_add(aux_stack->vars, vars_create('b', 69, 70, 71));
}
