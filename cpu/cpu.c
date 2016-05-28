#include <commons/config.h>
#include <commons/collections/list.h>
#include <commons/string.h>
#include <commons/log.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
//#include <parser/parser.h>
#include <parser/sintax.h>
#include <signal.h>
#include "protocolo_mensaje.h"
#include "pcb.h"
#include "cpu.h"
#include "parser.h" // parser/parser.h -> Modificado los typedef
#include "analizador.h"
#include "codigos_operacion.h"

int main(int argc, char** argv){

	// Declaro variables
	t_mensaje mensaje_recibido;
	int i_quantum;
	int quantum;
	unsigned tamano_pagina_umc;

	// Leer archivo config.conf
	leerArchivoConfig();

	// Asigno una funcion a la señal SIGUSR1
	signal(SIGUSR1, signal_sigusr1);

	// Me conecto a la UMC
	socketUMC = conectarseUMC();
	if(socketUMC == -1) abort();

	// Me conecto a el Nucleo
	socketNucleo = conectarseNucleo();
	if(socketNucleo == -1) abort();

	// Obtener tamaño de paginas UMC
	tamano_pagina_umc = obtenerTamanoPaginasUMC();

	while(1){
		// Recibo el PCB
		recibirMensajeNucleo(&mensaje_recibido);

		// Si no es el PCB, borro el mensaje
		if(mensaje_recibido.head.codigo != STRUCT_PCB){
			freeMensaje(&mensaje_recibido);
			continue;
		}

		// Recibir Quantum
		//quantum = recibirQuantum();

		// Convierto el mensaje en un PCB, y borro el mensaje
		pcb_global = mensaje_to_pcb(mensaje_recibido);
		freeMensaje(&mensaje_recibido);

		// Notifico a la UMC de cambio de proceso


		// LOOP QUANTUM
		for(i_quantum = 0; i_quantum < quantum; i_quantum++){

			// Obtener siguiente instruccion
			char *instruccion = obtenerSiguienteIntruccion();

			// analizadorLinea (parser)
			analizadorLinea(instruccion, &functions, &kernel_functions);

			// Libero memoria de la instruccion
			free(instruccion);

			// Actualizar PCB
			pcb_global.pc++;

		}
		// FIN LOOP QUANTUM

		// Notificar al Nucleo
		enviarPCBnucleo();

		// Notifico a la UMC de cambio de proceso

		// Libero recursos PCB
		freePCB(&pcb_global);

		// Si recibo señal para desconectarme, me desconecto
		if(notificacion_signal_sigusr1 == 1){
			close(socketUMC);
			close(socketNucleo);
			break;
		}
	}

	return EXIT_SUCCESS;
}

/*
 * leerArchivoConfig();
 * Parametros: -
 * Descripcion: Procedimiento que lee el archivo config.conf y lo carga en la variable infoConfig
 * Return: -
 */
void leerArchivoConfig() {

	t_config *config = config_create("config.conf");

	if (config == NULL) {
		free(config);
		abort();
	}
	// Guardo los datos en una variable global
	infoConfig.ip_nucleo = config_get_string_value(config, "IP_NUCLEO");
	infoConfig.puerto_nucleo = config_get_string_value(config, "PUERTO_NUCLEO");
	infoConfig.ip_umc = config_get_string_value(config, "IP_UMC");
	infoConfig.puerto_umc = config_get_string_value(config, "PUERTO_UMC");

	// No uso config_destroy(config) porque bugea
	free(config->path);
	free(config);
}

/*
 * conectarseUMC();
 * Parametros: -
 * Descripcion: Establece la conexion con la UMC
 * Return:
 * 		-> -1 :: Error
 * 		->  Other :: ID del socket
 */
int conectarseUMC() {
	int serverSocket = crearConexion(infoConfig.ip_umc, infoConfig.puerto_umc);
	if(serverSocket == -1){
		perror("Error al conectarse con UMC");
		return -1;
	}
	return serverSocket;
}

/*
 * conectarseNucleo();
 * Parametros: -
 * Descripcion: Establece la conexion con el Nucleo
 * Return: -
 * 		-> -1 :: Error
 * 		->  Other :: ID del socket
 */
int conectarseNucleo() {
	int serverSocket = crearConexion(infoConfig.ip_nucleo, infoConfig.puerto_nucleo);
	if(serverSocket == -1){
		perror("Error al conectarse con Nucleo");
		return -1;
	}
	return serverSocket;
}

/*
 * enviarMensajeUMC(char *mensaje);
 * Parametros:
 * 		-> mensaje: El mensaje a enviar
 * Descripcion: Envia un mensaje a la UMC
 * Return:
 * 		-> -1 :: Error
 * 		->  Other :: -
 */
int enviarMensajeUMC(t_mensaje mensaje) {
	int enviar = enviarMensaje(socketUMC,mensaje);
	if(enviar == -1){
		perror("Error al enviar mensaje a la UMC");
		return -1;
	}
	return enviar;
}

/*
 * enviarMensajeNucleo(char *mensaje);
 * Parametros:
 * 		-> mensaje: El mensaje a enviar
 * Descripcion: Envia un mensaje al Nucleo
 * Return:
 * 		-> -1 :: Error
 * 		->  Other :: -
 */
int enviarMensajeNucleo(t_mensaje mensaje) {
	int enviar = enviarMensaje(socketNucleo,mensaje);
	if(enviar == -1){
		perror("Error al enviar mensaje al Nucleo");
		return -1;
	}
	return enviar;
}

/*
 * recibirMensajeNucleo();
 * Parametros:
 * 		-> mensaje :: Donde se va a guardar el mensaje
 * Descripcion: Recibe un mensaje del Nucleo
 * Return:
 * 		-> -1 :: Error
 * 		->  Other :: -
 */
int recibirMensajeNucleo(t_mensaje *mensaje) {
	int recibir = recibirMensaje(socketNucleo,mensaje);
	if(recibir == -1){
		perror("Error al recibir mensaje del Nucleo");
		return -1;
	}
	return recibir;
}

/*
 * recibirMensajeUMC();
 * Parametros:
 * 		-> mensaje :: Donde se va a guardar el mensaje
 * Descripcion: Recibe un mensaje de la UMC
 * Return:
 * 		-> -1 :: Error
 * 		->  Other :: -
 */
int recibirMensajeUMC(t_mensaje *mensaje) {
	int recibir = recibirMensaje(socketUMC,mensaje);
	if(recibir == -1){
		perror("Error al recibir mensaje de la UMC");
		return -1;
	}
	return recibir;
}

/*
 * crearConexion();
 * Parametros:
 * 		-> ip :: Direccion IP donde nos vamos a conectar
 * 		-> puerto :: Puerto donde nos vamos a conetar
 * Descripcion: Crea una conxion con la IP y el Puerto dado
 * Return:
 * 		-> -1 :: Error
 * 		->  serverSocket :: ID del socket
 */
int crearConexion(const char *ip, const char *puerto) {
	struct addrinfo hints;
	struct addrinfo *serverInfo;
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	getaddrinfo(ip, puerto, &hints, &serverInfo);
	int serverSocket = socket(serverInfo->ai_family, serverInfo->ai_socktype, serverInfo->ai_protocol);
	if ((connect(serverSocket, serverInfo->ai_addr, serverInfo->ai_addrlen)) != 0) {
		return -1;
	}
	freeaddrinfo(serverInfo);
	return serverSocket;
}

/*
 * signal_sigusr1();
 * Parametros:
 * 		-> signal :: Identificador de Señal
 * Descripcion: Procedimiento que activa la bandera notificacion_signal_sigusr1 = 1;
 * Return: -
 */
void signal_sigusr1(int signal){
  printf("Recibida señal SIGUSR1.\n");
  notificacion_signal_sigusr1 = 1;
}


/*
 * obtenerSiguienteIntruccion();
 * Parametros: -
 * Descripcion: Busca en la UMC la proxima instruccion a ejecutarse segun el PCB GLOBAL
 * Return: Instruccion
 */
char *obtenerSiguienteIntruccion(){
	t_mensaje mensaje;
	unsigned parametros[3];
	parametros[0] = 0; // Numero de Pagina
	parametros[1] = pcb_global.indiceCodigo[0].offset_inicio; // Desplazamiento
	parametros[2] = pcb_global.indiceCodigo[0].offset_fin; // Cantidad de bytes a leer
	mensaje.head.codigo = GET_DATA;
	mensaje.head.cantidad_parametros = 3;
	mensaje.head.tam_extra = 0;
	mensaje.parametros = parametros;
	mensaje.mensaje_extra = NULL;

	// Envio al UMC la peticion
	enviarMensajeUMC(mensaje);

	// Libero memoria de mensaje
	freeMensaje(&mensaje);

	// Recibo mensaje
	recibirMensajeUMC(&mensaje);

	if(mensaje.head.codigo != RETURN_DATA){
		// ERROR
	}

	char *instruccion = malloc(mensaje.head.tam_extra);
	memcpy(instruccion, mensaje.mensaje_extra, mensaje.head.tam_extra);

	// Libero memoria de mensaje
	freeMensaje(&mensaje);

	return instruccion;
}

/*
 * obtenerTamanoPaginasUMC();
 * Parametros: -
 * Descripcion: Peticion a la UMC para que devuelva el tamaño de pagina
 * Return: tamano_pagina
 */
unsigned obtenerTamanoPaginasUMC(){
	t_mensaje mensaje;
	unsigned tamano_pagina;
	mensaje.head.codigo = GET_TAM_PAGINA;
	mensaje.head.cantidad_parametros = 0;
	mensaje.head.tam_extra = 0;
	mensaje.parametros = NULL;
	mensaje.mensaje_extra = NULL;

	// Envio al UMC la peticion
	enviarMensajeUMC(mensaje);

	// Libero memoria de mensaje
	freeMensaje(&mensaje);

	// Recibo mensaje
	recibirMensajeUMC(&mensaje);

	if(mensaje.head.codigo != RETURN_TAM_PAGINA){
		// ERROR
	}

	// Tamaño de pagina
	tamano_pagina = mensaje.parametros[0];

	// Libero memoria de mensaje
	freeMensaje(&mensaje);

	return tamano_pagina;
}

void enviarPCBnucleo(){
	t_mensaje mensaje;
	mensaje = pcb_to_mensaje(pcb_global,0);
	enviarMensajeNucleo(mensaje);
	freeMensaje(&mensaje);
}

/*
 * FIN CPU.c
 */

/*
 * INICIO PROTOCOLO_MENSAJE.c
 */

/*
 * Hipotesis: Al trabajar todos con la misma maquina virtual, no tenemos en cuenta la variacion de tamaño de los tipo de datos segun las arquitecturas.
 */
/*
 * empaquetar_mensaje();
 * Parametros:
 *		-> mensaje: Bloque de mensaje a empaquetar
 * Descripcion: Dado un mensaje, lo empaqueta para enviarlo
 * Return: *mensaje_empaquetado
 */
void *empaquetar_mensaje(t_mensaje mensaje) {

	// Variables usadas
	unsigned desplazamiento = 0;
	unsigned i_parametro;

	// Creo un bloque en memoria con la tamaño del Head + Payload
	void *mensaje_empaquetado = malloc(sizeof(t_mensajeHead) + sizeof(unsigned) * mensaje.head.cantidad_parametros + mensaje.head.tam_extra);

	// Copio el head en el bloque de memoria creado
	memcpy(mensaje_empaquetado + desplazamiento, &mensaje.head.codigo, sizeof(unsigned));
	desplazamiento += sizeof(unsigned);
	memcpy(mensaje_empaquetado + desplazamiento, &mensaje.head.cantidad_parametros, sizeof(unsigned));
	desplazamiento += sizeof(unsigned);
	memcpy(mensaje_empaquetado + desplazamiento, &mensaje.head.tam_extra, sizeof(unsigned));
	desplazamiento += sizeof(unsigned);

	// Copio los parametros
	for (i_parametro = 0; i_parametro < mensaje.head.cantidad_parametros; i_parametro++){
		memcpy(mensaje_empaquetado + desplazamiento, &mensaje.parametros[i_parametro], sizeof(unsigned));
		desplazamiento += sizeof(unsigned);
	}

	// Copio el mensaje_extra
	memcpy(mensaje_empaquetado + desplazamiento, mensaje.mensaje_extra, mensaje.head.tam_extra);

	// Devuelvo
	return mensaje_empaquetado;
}

/*
 * desempaquetar_head();
 * Parametros:
 *		-> buffer: Bytes a desempaquetar
 * Descripcion: Dado un buffer, desempaqueta el HEAD
 * Return: t_mensajeHead mensaje_head
 */
t_mensajeHead desempaquetar_head(const void *buffer) {

	// Declaro variables usadas
	unsigned desplazamiento = 0;
	t_mensajeHead mensaje_head;

	// Desempaqueto el head
	memcpy(&mensaje_head.codigo, buffer, sizeof(unsigned));
	desplazamiento += sizeof(unsigned);
	memcpy(&mensaje_head.cantidad_parametros, buffer + desplazamiento, sizeof(unsigned));
	desplazamiento += sizeof(unsigned);
	memcpy(&mensaje_head.tam_extra, buffer + desplazamiento, sizeof(unsigned));

	return mensaje_head;
}

/*
 * desempaquetar_mensaje();
 * Parametros:
 *		-> buffer: Bytes a desempaquetar
 * Descripcion: Dado un buffer, lo desempaqueta
 * Return: t_mensaje mensaje_desempaquetado
 */
t_mensaje desempaquetar_mensaje(const void *buffer) {

	// Declaro variables usadas
	unsigned i_parametro;
	unsigned desplazamiento = 0;
	t_mensaje mensaje_desempaquetado;

	// Desempaqueto el HEAD
	mensaje_desempaquetado.head = desempaquetar_head(buffer);
	desplazamiento = sizeof(t_mensajeHead);

	// Creo memoria para los parametros
	mensaje_desempaquetado.parametros = malloc(sizeof(unsigned) * mensaje_desempaquetado.head.cantidad_parametros);

	// Desempaqueto los parametros
	for (i_parametro = 0; i_parametro < mensaje_desempaquetado.head.cantidad_parametros; i_parametro++){
		memcpy(&mensaje_desempaquetado.parametros[i_parametro], buffer + desplazamiento, sizeof(unsigned));
		desplazamiento += sizeof(unsigned);
	}

	// Desempaqueto lo extra
	mensaje_desempaquetado.mensaje_extra = malloc(mensaje_desempaquetado.head.tam_extra);
	memcpy(mensaje_desempaquetado.mensaje_extra, buffer + desplazamiento, mensaje_desempaquetado.head.tam_extra);

	return mensaje_desempaquetado;
}

/*
 * enviarMensaje();
 * Parametros:
 * 	-> serverSocket :: ID del socket donde voy a enviar el mensaje
 * 	-> mensaje	:: Mensaje a enviar
 * Descripcion: Envia un mensaje a traves serverSocket
 * Return:
 * 		-> -1 :: Error
 * 		->  other :: -
 */
int enviarMensaje(int serverSocket, t_mensaje mensaje){

	void *mensaje_empaquetado = empaquetar_mensaje(mensaje);
	unsigned tamano_mensaje = sizeof(unsigned)*3 + sizeof(unsigned) * mensaje.head.cantidad_parametros + mensaje.head.tam_extra;

	int enviar = send(serverSocket, mensaje_empaquetado, tamano_mensaje, MSG_NOSIGNAL);

	freeMensaje(mensaje_empaquetado);

	return enviar;

}

/*
 * recibirMensaje();
 * Parametros:
 * 		-> serverSocket :: ID del socket desde donde voy a recibir el mensaje
 * 		-> mensaje :: Donde voy a guardar el mensaje
 * 		-> tamano :: Tamaño que ocupa el mensaje
 * Descripcion: Recibe un mensaje del serverSocket y lo guarda en mensaje
 * Return:
 * 		-> -1 :: Error
 * 		->  other :: -
 */
int recibirMensaje(int serverSocket, t_mensaje *mensaje){

	// Declaro variables
	t_mensajeHead mensaje_head;
	unsigned desplazamiento = 0;
	int recibir;

	// Recibo el HEAD
	void *buffer_head = malloc(sizeof(unsigned)*3);
	recibir = recibirBytes(serverSocket, buffer_head, sizeof(unsigned)*3);
	if (recibir <= 0){
		free(buffer_head);
		return recibir;
	}

	// Obtengo los valores del HEAD
	mensaje_head = desempaquetar_head(buffer_head);
	desplazamiento += sizeof(t_mensajeHead);

	// Me preparo para recibir el Payload
	unsigned faltan_recibir = sizeof(unsigned) * mensaje_head.cantidad_parametros + mensaje_head.tam_extra;
	void *bufferTotal = malloc(sizeof(t_mensajeHead) + faltan_recibir);
	memcpy(bufferTotal, buffer_head, sizeof(t_mensajeHead));

	// Recibo el Payload
	recibir = recibirBytes(serverSocket, bufferTotal + desplazamiento, faltan_recibir);

	// Desempaqueto el mensaje
	*mensaje = desempaquetar_mensaje(bufferTotal);

	// Limpieza
	free(buffer_head);
	free(bufferTotal);

	//
	return recibir;
}

/*
 * recibirBytes();
 * Parametros:
 * 		-> serverSocket :: ID del socket desde donde voy a recibir el mensaje
 * 		-> buffer :: Donde voy a guardar el mensaje
 * 		-> tamano :: Tamaño que ocupa el mensaje
 * Descripcion: Recibe un mensaje del serverSocket y lo guarda en buffer
 * Return:
 * 		-> -1 :: Error
 * 		->  other :: -
 */

int recibirBytes(int serverSocket, void *buffer, unsigned tamano){
	int recibir = recv(serverSocket, buffer, tamano, MSG_WAITALL);
	return recibir;
}

/*
 * freeMensaje();
 * Parametros:
 * 		-> mensaje :: Puntero a mensaje a destruir
 * Descripcion: Procedimiento que libera memoria de un mensaje
 * Return: -
 */

void freeMensaje(t_mensaje *mensaje) {
  free(mensaje->parametros);
  free(mensaje->mensaje_extra);
}

void testMensajeProtocolo(){
	// Declaro variables usadas
	t_mensaje mensaje;
	t_mensaje mensaje_new;
	unsigned parametros[2];

	// Defino el mensaje a empaquetar
	mensaje.head.codigo = 20;
	mensaje.head.cantidad_parametros = 2;
	mensaje.head.tam_extra = 0;
	mensaje.mensaje_extra = NULL;
	mensaje.parametros = parametros;
	parametros[0] = 3;
	parametros[1] = 7;

	//
	const char *buffer = empaquetar_mensaje(mensaje);

	mensaje_new = desempaquetar_mensaje(buffer);

	printf("%i",mensaje_new.head.codigo);
}

/*
 * FIN PROTOCOLO_MENSAJE.c
 */

/*
 * INICIO PRIMITIVAS.c
 */

/*
 * FUNCIONES ANALIZADOR
 *
 */
/*
 * Hipotesis: Consideramos que no hay errores semanticos ni de sintaxis
 *
 */

int _is_variableX(t_variable *tmp) {
	return tmp->identificador == nombreVariable_aBuscar;
}

t_puntero definirVariable(t_nombre_variable variable) {
	int lastStack = list_size(pcb_global.indiceStack);
	t_indiceStack *aux_stack;

	// Crear variable en la memoria (Reservar el espacio)
	t_posicionDeMemoria posicionMemoria;
	t_mensaje mensaje;
	unsigned parametros[1];
	parametros[0] = sizeof(t_valor_variable);	// Tamaño a reservar
	mensaje.head.codigo = RESERVE_MEMORY;
	mensaje.head.cantidad_parametros = 1;
	mensaje.head.tam_extra = 0;
	mensaje.parametros = parametros;
	mensaje.mensaje_extra = NULL;

	// Envio al UMC la peticion
	enviarMensajeUMC(mensaje);

	// Libero memoria de mensaje
	freeMensaje(&mensaje);

	// Recibo mensaje
	recibirMensajeUMC(&mensaje);

	if(mensaje.head.codigo != RETURN_POS){
		// ERROR
	}

	posicionMemoria.numeroPagina = mensaje.parametros[0];
	posicionMemoria.offset = mensaje.parametros[1];
	posicionMemoria.size = mensaje.parametros[2];

	// Libero memoria de mensaje
	freeMensaje(&mensaje);

	// Registrar variable en el ultimo Indice Stack
	aux_stack = list_get(pcb_global.indiceStack, lastStack);
	list_add(aux_stack->vars, vars_create(variable, posicionMemoria.numeroPagina, posicionMemoria.offset, posicionMemoria.size));

	// Devuelvo posicion de la variable en el contexto actual
	return posicionMemoria;
}

t_puntero obtenerPosicionVariable(t_nombre_variable variable) {
	nombreVariable_aBuscar = variable;
	int lastStack = list_size(pcb_global.indiceStack);
	t_puntero puntero_variable;
	t_indiceStack *aux_stack;
	t_variable *aux_vars;

	// Ingresar al ultimo indice stack
	aux_stack = list_get(pcb_global.indiceStack, lastStack);
	aux_vars = list_find(aux_stack->vars, (void*) _is_variableX);

	puntero_variable = aux_vars->posicionMemoria;

	return puntero_variable;
}

t_valor_variable dereferenciar(t_puntero puntero) {

	// Variables
	t_mensaje mensaje;
	unsigned parametros[3];
	parametros[0] = puntero.numeroPagina;	// Numero de Pagina
	parametros[1] = puntero.offset;			// Desplazamiento
	parametros[2] = puntero.size;			// Cantidad de bytes a leer
	mensaje.head.codigo = GET_DATA;
	mensaje.head.cantidad_parametros = 3;
	mensaje.head.tam_extra = 0;
	mensaje.parametros = parametros;
	mensaje.mensaje_extra = NULL;

	// Envio al UMC la peticion
	enviarMensajeUMC(mensaje);

	// Libero memoria de mensaje
	freeMensaje(&mensaje);

	// Recibo mensaje
	recibirMensajeUMC(&mensaje);

	if(mensaje.head.codigo != RETURN_DATA){
		// ERROR
	}

	t_valor_variable *contenido_tmp = malloc(mensaje.head.tam_extra);
	memcpy(contenido_tmp, mensaje.mensaje_extra, mensaje.head.tam_extra);

	t_valor_variable contenido_variable = (*contenido_tmp);

	// Libero memoria de mensaje
	freeMensaje(&mensaje);
	free(contenido_tmp);

	return contenido_variable;
}

void asignar(t_puntero puntero, t_valor_variable variable) {
	// Variables
	t_mensaje mensaje;
	unsigned parametros[4];
	parametros[0] = puntero.numeroPagina;	// Numero de Pagina
	parametros[1] = puntero.offset;			// Desplazamiento
	parametros[2] = puntero.size;			// Tamaño
	parametros[3] = variable;				// Data
	mensaje.head.codigo = RECORD_DATA;
	mensaje.head.cantidad_parametros = 4;
	mensaje.head.tam_extra = 0;
	mensaje.parametros = parametros;
	mensaje.mensaje_extra = NULL;

	// Envio al UMC la peticion
	enviarMensajeUMC(mensaje);

	// Libero memoria de mensaje
	freeMensaje(&mensaje);

	// Recibo mensaje
	recibirMensajeUMC(&mensaje);

	if(mensaje.head.codigo != RECORD_OK){
		// ERROR
	}

	// Libero memoria de mensaje
	freeMensaje(&mensaje);
}

// t_valor_variable obtenerValorCompartida(t_nombre_compartida variable);
// t_valor_variable asignarValorCompartida(t_nombre_compartida variable, t_valor_variable valor);
// t_puntero_instruccion irAlLabel(t_nombre_etiqueta etiqueta);
// t_puntero_instruccion llamarFuncion(t_nombre_etiqueta etiqueta, t_posicion donde_retornar, t_puntero_instruccion linea_en_ejecucion);
// t_puntero_instruccion retornar(t_valor_variable retorno);

void imprimir(t_valor_variable valor) {
	printf("Imprimir %d\n", valor);
}

void imprimirTexto(char* texto) {
	printf("ImprimirTexto: %s", texto);
}

// void entradaSalida(t_nombre_dispositivo dispositivo, int tiempo);
// void wait(t_nombre_semaforo identificador_semaforo);
// void signal(t_nombre_semaforo identificador_semaforo);

/*
 * FIN PRIMITIVAS.c
 */
/*
 * INICIO PCB.c
 */
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
/*
 * FIN PCB.c
 */
