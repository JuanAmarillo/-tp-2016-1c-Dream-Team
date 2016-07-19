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
#include <signal.h>
#include "protocolo_mensaje.h"
#include "pcb.h"
#include "cpu.h"
#include <parser/parser.h>
#include <parser/metadata_program.h>
#include "analizador.h"
#include "messageCode.h"

int main(int argc, char** argv){

	// Declaro variables
	t_mensaje mensaje_recibido;
	int i_quantum;
	int quantum;
	int quantum_sleep;

	// Inicializo Log
	logger = log_create("log.txt", "CPU", 1, LOG_LEVEL_TRACE);

	// Leer archivo config.conf
	leerArchivoConfig();

	// Asigno una funcion a la señal SIGUSR1
	signal(SIGUSR1, signal_sigusr1);

	// Me conecto a la UMC
	socketUMC = conectarseUMC();

	// Me conecto a el Nucleo
	//socketNucleo = socketUMC;
	socketNucleo = conectarseNucleo();

	// Obtener tamaño de paginas UMC
	tamano_pagina_umc = obtenerTamanoPaginasUMC();

	while(1){


		log_trace(logger, "================================");
		log_trace(logger, "Esperando PCB");
		log_trace(logger, "================================");

		// Seteo estado en ejecucion
		estado_ejecucion = 0;
		quantum = 0;
		quantum_sleep = 0;

		// Recibo el PCB
		recibirMensajeNucleo(&mensaje_recibido);

		// Si no es el PCB, borro el mensaje
		if(mensaje_recibido.head.codigo != STRUCT_PCB){
			log_error(logger, "Se recibio un mensaje diferente a 'STRUCT_PCB'");
			freeMensaje(&mensaje_recibido);
			abort();
		}

		// Recibir Quantum
		recibirQuantum(&quantum, &quantum_sleep);
		log_trace(logger, "Se recibio quantum: %u ; quantum_sleep: %u", quantum, quantum_sleep);

		// Convierto el mensaje en un PCB, y borro el mensaje
		pcb_global = mensaje_to_pcb(mensaje_recibido);
		freeMensaje(&mensaje_recibido);

		// Notifico a la UMC de cambio de proceso
		notificarCambioProceso();

		// LOOP QUANTUM
		for(i_quantum = 1; i_quantum <= quantum; i_quantum++){

			// Obtener siguiente instruccion
			char *instruccion = obtenerSiguienteIntruccion();

			log_trace(logger, "PID: %u; PC: %u; Quantum %u de %u -> '%s'", pcb_global.pid, pcb_global.pc, i_quantum, quantum, instruccion);

			// Actualizar PCB
			pcb_global.pc++;

			// analizadorLinea (parser)
			analizadorLinea(instruccion, &functions, &kernel_functions);

			// Libero memoria de la instruccion
			free(instruccion);

			// Sleep
			usleep(quantum_sleep);
			//getchar();

			// Realizo acciones segun el estado de ejecucion
			if((estado_ejecucion != 0) || (notificacion_signal_sigusr1 == 1)) break;
		}
		// FIN LOOP QUANTUM

		switch (estado_ejecucion) {
		      case 0: // Finalizo el Quantum normalmente
		    	  log_trace(logger, "Finalizo el Quantum Normalmente");
		    	  enviarPCBnucleo(STRUCT_PCB);
		        break;
		      case 1: // Finalizo el programa
		    	  log_trace(logger, "Finalizo el programa");
		    	  enviarPCBnucleo(STRUCT_PCB_FIN);
		        break;
		      case 2:
		    	  log_trace(logger, "Ocurrio Segmentation Fault");
		    	  enviarPCBnucleo(STRUCT_PCB_FIN_ERROR);
		    	break;
		      case 3:
		    	  log_trace(logger, "Ocurrio IO");
		    	  enviarPCBnucleo(STRUCT_PCB_IO);
		    	break;
		      case 4:
		    	  log_trace(logger, "Finalizo por WAIT");
		    	  enviarPCBnucleo(STRUCT_PCB_WAIT);
		    	break;
		      default: // Otro error
		    	break;
		}

		// Libero recursos PCB
		freePCB(&pcb_global);

		// Si recibo señal para desconectarme, me desconecto
		if(notificacion_signal_sigusr1 == 1){
			close(socketUMC);
			close(socketNucleo);
			break;
		}
	}

	log_trace(logger, "Fin CPU");
	log_destroy(logger);

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
		log_error(logger, "Error al leer archivo config.conf");
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
		log_error(logger, "No se pudo conectarse con la UMC");
		abort();
	}
	log_trace(logger, "UMC Conectada");
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
		log_error(logger, "No se pudo conectarse con el Nucleo");
		abort();
	}
	log_trace(logger, "Nucleo Conectado");
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
void enviarMensajeUMC(t_mensaje mensaje) {
	int enviar = enviarMensaje(socketUMC,mensaje);
	if(enviar <= 0){
		log_error(logger, "Se desconecto la UMC.");
		abort();
	}
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
void enviarMensajeNucleo(t_mensaje mensaje) {
	int enviar = enviarMensaje(socketNucleo,mensaje);
	if(enviar <= 0){
		log_error(logger, "Se desconecto el Nucleo.");
		abort();
	}
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
void recibirMensajeNucleo(t_mensaje *mensaje) {
	int recibir = recibirMensaje(socketNucleo,mensaje);
	if(recibir <= 0){
		log_error(logger, "Se desconecto el Nucleo.");
		abort();
	}
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
void recibirMensajeUMC(t_mensaje *mensaje) {
	int recibir = recibirMensaje(socketUMC,mensaje);
	if(recibir <= 0){
		log_error(logger, "Se desconecto la UMC.");
		abort();
	}
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
  log_trace(logger, "signal_sigusr1();");
  notificacion_signal_sigusr1 = 1;
}


/*
 *  _esEspacio_cpu();
 */
int _esEspacio_cpu(char caracter){
	if(caracter==' ' || caracter=='\t' || caracter=='\f' || caracter=='\r' || caracter=='\v'){
		return 1;
	} else {
		return 0;
	}
}

/*
 * _string_trim_cpu();
 */
char* _string_trim_cpu(char* texto){
    int i;
    while (_esEspacio_cpu(*texto) == 1) texto++;   //Anda a el primer no-espacio
    for (i = strlen (texto) - 1; i>0 && (_esEspacio_cpu(texto[i]) == 1); i--);   //y de atras para adelante
    texto[i + 1] = '\0';
    return texto;
}


/*
 * obtenerSiguienteIntruccion();
 * Parametros: -
 * Descripcion: Busca en la UMC la proxima instruccion a ejecutarse segun el PCB GLOBAL
 * Return: Instruccion
 */
char *obtenerSiguienteIntruccion(){
	log_trace(logger, "obtenerSiguienteInstruccion();");
	t_mensaje mensaje;
	unsigned parametros[3];

	unsigned num_pagina = (pcb_global.indiceCodigo[pcb_global.pc].offset_inicio)/(tamano_pagina_umc);
	//
	parametros[0] = num_pagina; // Numero de Pagina
	parametros[1] = pcb_global.indiceCodigo[pcb_global.pc].offset_inicio - tamano_pagina_umc * num_pagina; // Desplazamiento
	parametros[2] = pcb_global.indiceCodigo[pcb_global.pc].offset_fin; // Cantidad de bytes a leer
	mensaje.head.codigo = GET_DATA;
	mensaje.head.cantidad_parametros = 3;
	mensaje.head.tam_extra = 0;
	mensaje.parametros = parametros;
	mensaje.mensaje_extra = NULL;


	log_trace(logger, "Inicio: %i Tamanio: %i", pcb_global.indiceCodigo[pcb_global.pc].offset_inicio, pcb_global.indiceCodigo[pcb_global.pc].offset_fin);

	// Envio al UMC la peticion
	enviarMensajeUMC(mensaje);

	// Recibo mensaje
	recibirMensajeUMC(&mensaje);

	if(mensaje.head.codigo != RETURN_OK){
		log_error(logger, "Se recibio un mensaje diferente a 'RETURN_OK'");
		abort();
	}

	int i;
	for(i=0; mensaje.mensaje_extra[i] != '\0'; i++){
		if( mensaje.mensaje_extra[i] == '\n' ){
			mensaje.mensaje_extra[i] = '\0';
			break;
		}
	}


	char *tmp = _string_trim_cpu(mensaje.mensaje_extra);
	char *tmp2 = malloc(strlen(tmp)+1);
	memcpy(tmp2, tmp, strlen(tmp));
	memset(tmp2+strlen(tmp),'\0',1);

	free(mensaje.mensaje_extra);

	return tmp2;
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

	// Recibo mensaje
	recibirMensajeUMC(&mensaje);

	if(mensaje.head.codigo != RETURN_TAM_PAGINA){
		log_error(logger, "Se recibio un mensaje diferente a 'RETURN_TAM_PAGINA'");
		abort();
	}

	// Tamaño de pagina
	tamano_pagina = mensaje.parametros[0];

	// Libero memoria de mensaje
	freeMensaje(&mensaje);

	log_trace(logger, "Tamanio de pagina: %u", tamano_pagina);

	return tamano_pagina;
}

/*
 * enviarPCBnucleo();
 * Parametros: -
 * Descripcion: Envia el PCB global al nucleo
 * Return: -
 */
void enviarPCBnucleo(unsigned codigo){
	t_mensaje mensaje;
	mensaje = pcb_to_mensaje(pcb_global, codigo);
	enviarMensajeNucleo(mensaje);
	freeMensaje(&mensaje);
}

/*
 * recibirQuantum();
 * Parametros: -
 * Descripcion: Obtiene la cantidad de quantum a ejecutar
 * Return: cantidad de quantum
 */
void recibirQuantum(int *quantum, int *quantum_sleep){
	t_mensaje mensaje;

	// Recibo mensaje
	recibirMensajeNucleo(&mensaje);

	if(mensaje.head.codigo != QUANTUM){
		log_error(logger, "Se recibio un mensaje diferente a 'QUANTUM'");
		abort();
	}

	// Tamaño de pagina
	*quantum = mensaje.parametros[0];
	*quantum_sleep = mensaje.parametros[1];

	// Libero memoria de mensaje
	freeMensaje(&mensaje);

}

void notificarCambioProceso(){
	t_mensaje mensaje;
	unsigned parametros[1];
	parametros[0] = pcb_global.pid;
	mensaje.head.codigo = CAMBIO_PROCESO;
	mensaje.head.cantidad_parametros = 1;
	mensaje.head.tam_extra = 0;
	mensaje.parametros = parametros;
	mensaje.mensaje_extra = NULL;

	// Envio al UMC la peticion
	enviarMensajeUMC(mensaje);
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
	if(mensaje.mensaje_extra > 0) memcpy(mensaje_empaquetado + desplazamiento, mensaje.mensaje_extra, mensaje.head.tam_extra);

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
	if(mensaje_desempaquetado.head.tam_extra > 0) memcpy(mensaje_desempaquetado.mensaje_extra, buffer + desplazamiento, mensaje_desempaquetado.head.tam_extra);

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
	unsigned tamano_mensaje = sizeof(t_mensajeHead) + sizeof(unsigned) * mensaje.head.cantidad_parametros + mensaje.head.tam_extra;

	int enviar = send(serverSocket, mensaje_empaquetado, tamano_mensaje, MSG_NOSIGNAL);

	free(mensaje_empaquetado);

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
	if(faltan_recibir > 0) recibir = recibirBytes(serverSocket, bufferTotal + desplazamiento, faltan_recibir);

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

t_puntero posToPuntero(t_posicionDeMemoria posicionMemoria){
	return tamano_pagina_umc * posicionMemoria.numeroPagina + posicionMemoria.offset;
}

t_posicionDeMemoria punteroToPos(t_puntero puntero){
	t_posicionDeMemoria posicionMemoria;
	posicionMemoria.numeroPagina = puntero/tamano_pagina_umc;
	posicionMemoria.offset = puntero - tamano_pagina_umc * posicionMemoria.numeroPagina;
	posicionMemoria.size = sizeof(t_puntero);
	return posicionMemoria;
}

t_puntero parser_definirVariable(t_nombre_variable identificador_variable) {
	log_trace(logger, "--> parser_definirVariable(%c);", identificador_variable);
	t_indiceStack *aux_stack;
	t_variable *aux_vars;
	t_posicionDeMemoria posicionMemoria;
	t_puntero puntero_tmp;
	int cantidad_vars;
	unsigned sp_tmp = pcb_global.sp;

	// Obtengo la ultima direccion de memoria que se encuentra guardada
	while(1){
		aux_stack = list_get(pcb_global.indiceStack, sp_tmp);
		if(sp_tmp == 0){
			cantidad_vars = list_size(aux_stack->vars);
			if(cantidad_vars == 0){
				aux_vars = malloc(sizeof(t_variable));
				aux_vars->posicionMemoria.numeroPagina = pcb_global.cantidadPaginas-1;
				aux_vars->posicionMemoria.offset = pcb_global.indiceCodigo[pcb_global.total_instrucciones-1].offset_inicio + pcb_global.indiceCodigo[pcb_global.total_instrucciones-1].offset_fin;
				aux_vars->posicionMemoria.size = 0;
			} else {
				aux_vars = list_get(aux_stack->vars, cantidad_vars-1);
			}
		} else {
			cantidad_vars = list_size(aux_stack->vars);
			if(cantidad_vars == 0){ sp_tmp--; continue; }
			aux_vars = list_get(aux_stack->vars, cantidad_vars-1);
		}
		break;
	}

	// Calculo la direccion de memoria
	puntero_tmp = posToPuntero(aux_vars->posicionMemoria);
	puntero_tmp = puntero_tmp + sizeof(t_valor_variable);
	posicionMemoria = punteroToPos(puntero_tmp);

	// Registrar variable en el ultimo Indice Stack
	aux_stack = list_get(pcb_global.indiceStack, pcb_global.sp);
	list_add(aux_stack->vars, vars_create(identificador_variable, posicionMemoria.numeroPagina, posicionMemoria.offset, posicionMemoria.size));

	// Devuelvo posicion de la variable en el contexto actual
	log_trace(logger, "----> Return: %c: (%u,%u,%u)", identificador_variable, posicionMemoria.numeroPagina, posicionMemoria.offset, posicionMemoria.size);

	t_puntero puntero_r = posToPuntero(posicionMemoria);
	return puntero_r;
}

t_puntero parser_obtenerPosicionVariable(t_nombre_variable identificador_variable) {
	log_trace(logger, "--> parser_obtenerPosicionVariable(%c);", identificador_variable);
	nombreVariable_aBuscar = identificador_variable;

	t_posicionDeMemoria puntero_variable;
	t_indiceStack *aux_stack;
	t_variable *aux_vars;

	// Ingresar al ultimo indice stack
	aux_stack = list_get(pcb_global.indiceStack, pcb_global.sp);
	aux_vars = list_find(aux_stack->vars, (void*) _is_variableX);

	puntero_variable = aux_vars->posicionMemoria;
	log_trace(logger, "----> Return: %c: (%u,%u,%u)", identificador_variable, puntero_variable.numeroPagina, puntero_variable.offset, puntero_variable.size);

	return posToPuntero(puntero_variable);
}

t_valor_variable parser_dereferenciar(t_puntero direccion_variable_puntero) {
	t_posicionDeMemoria direccion_variable = punteroToPos(direccion_variable_puntero);
	log_trace(logger, "--> parser_dereferenciar((%u,%u,%u));", direccion_variable.numeroPagina, direccion_variable.offset, direccion_variable.size);
	// Variables
	t_valor_variable contenido_variable;
	t_mensaje mensaje;
	unsigned parametros[3];
	parametros[0] = direccion_variable.numeroPagina;	// Numero de Pagina
	parametros[1] = direccion_variable.offset;			// Desplazamiento
	parametros[2] = direccion_variable.size;			// Cantidad de bytes a leer
	mensaje.head.codigo = GET_DATA;
	mensaje.head.cantidad_parametros = 3;
	mensaje.head.tam_extra = 0;
	mensaje.parametros = parametros;
	mensaje.mensaje_extra = NULL;

	// Envio al UMC la peticion
	enviarMensajeUMC(mensaje);

	// Recibo mensaje
	recibirMensajeUMC(&mensaje);

	if(mensaje.head.codigo != RETURN_OK){
		log_error(logger, "Se recibio un mensaje diferente a 'RETURN_OK'");
		abort();
	}

	if(mensaje.parametros[0] != 1){
		estado_ejecucion = 2; // Fuera de segmento
		freeMensaje(&mensaje);
		return contenido_variable;
	}

	contenido_variable = *((int*) mensaje.mensaje_extra);

	// Libero memoria de mensaje
	freeMensaje(&mensaje);
	log_trace(logger, "----> Return: %i", contenido_variable, contenido_variable);
	return contenido_variable;
}

void parser_asignar(t_puntero direccion_variable_puntero, t_valor_variable valor) {
	t_posicionDeMemoria direccion_variable = punteroToPos(direccion_variable_puntero);
	log_trace(logger, "--> parser_asignar((%u,%u,%u),%i);", direccion_variable.numeroPagina, direccion_variable.offset, direccion_variable.size, valor);
	// Variables
	t_mensaje mensaje;
	unsigned parametros[4];
	parametros[0] = direccion_variable.numeroPagina;	// Numero de Pagina
	parametros[1] = direccion_variable.offset;			// Desplazamiento
	parametros[2] = direccion_variable.size;			// Tamaño
	parametros[3] = valor;								// Data (MARCO_REVISAR) valor es int
	mensaje.head.codigo = RECORD_DATA;
	mensaje.head.cantidad_parametros = 4;
	mensaje.head.tam_extra = 0;
	mensaje.parametros = parametros;
	mensaje.mensaje_extra = NULL;

	// Envio al UMC la peticion
	enviarMensajeUMC(mensaje);

	// Recibo mensaje
	recibirMensajeUMC(&mensaje);

	if(mensaje.head.codigo != RETURN_OK){
		log_error(logger, "Se recibio un mensaje diferente a 'RETURN_OK'");
		abort();
	}

	if(mensaje.parametros[0] != 1){
		estado_ejecucion = 3; // Fuera de segmento
	}

	// Libero memoria de mensaje
	freeMensaje(&mensaje);
}

t_valor_variable parser_obtenerValorCompartida(t_nombre_compartida variable){
	log_trace(logger, "--> parser_obtenerValorCompartida(%s);", variable);

	// Variables
	t_mensaje mensaje;
	mensaje.head.codigo = OBTENER_COMPARTIDA;
	mensaje.head.cantidad_parametros = 0;
	mensaje.head.tam_extra = strlen(variable) + 1;
	mensaje.parametros = NULL;
	mensaje.mensaje_extra = variable;

	// Envio al UMC la peticion
	enviarMensajeNucleo(mensaje);

	// Libero memoria de mensaje
	freeMensaje(&mensaje);

	// Recibo mensaje
	recibirMensajeNucleo(&mensaje);

	//
	if(mensaje.head.codigo != RETURN_OBTENER_COMPARTIDA){
		log_error(logger, "Se recibio un mensaje diferente a 'RETURN_OBTENER_COMPARTIDA'");
		abort();
	}

	t_valor_variable valor = mensaje.parametros[0];

	// Libero memoria de mensaje
	freeMensaje(&mensaje);
	log_trace(logger, "----> return: %i", valor);
	return valor;
}

t_valor_variable parser_asignarValorCompartida(t_nombre_compartida variable, t_valor_variable valor){
	log_trace(logger, "--> parser_asignarValorCompartida(%s, %i);", variable, valor);

	// Variables
	t_mensaje mensaje;
	unsigned parametros[1];
	parametros[0] = valor;
	mensaje.head.codigo = ASIGNAR_COMPARTIDA;
	mensaje.head.cantidad_parametros = 1;
	mensaje.head.tam_extra = strlen(variable) + 1;
	mensaje.parametros = parametros;
	mensaje.mensaje_extra = variable;

	// Envio al UMC la peticion
	enviarMensajeNucleo(mensaje);

	// Libero memoria de mensaje
	freeMensaje(&mensaje);
	log_trace(logger, "----> return: %i", valor);
	return valor;
}

void parser_irAlLabel(t_nombre_etiqueta etiqueta){
	log_trace(logger, "--> parser_irAlLabel(%s);", etiqueta);
	unsigned intruction = metadata_buscar_etiqueta(etiqueta, pcb_global.indiceEtiquetas, pcb_global.tam_indiceEtiquetas);
	pcb_global.pc = intruction;
	log_trace(logger, "----> PC: %u", pcb_global.pc);
}

void parser_llamarSinRetorno(t_nombre_etiqueta etiqueta){
	log_trace(logger, "--> parser_llamarSinRetorno(%s);", etiqueta);
	// Busco PC de la primera instruccion de la funcion
	unsigned pc_first_intruction = metadata_buscar_etiqueta(etiqueta, pcb_global.indiceEtiquetas, pcb_global.tam_indiceEtiquetas);

	// Creo nuevo contexto, y guardo retPos
	list_add(pcb_global.indiceStack, stack_create(pcb_global.pc, 0, 0, 0));
	pcb_global.sp++;

	// Cambio el PC actual
	pcb_global.pc = pc_first_intruction;

	log_trace(logger, "----> Return: %u", pcb_global.pc);
}

void parser_llamarConRetorno(t_nombre_etiqueta etiqueta, t_puntero donde_retornar_puntero){
	t_posicionDeMemoria donde_retornar = punteroToPos(donde_retornar_puntero);
	log_trace(logger, "--> parser_llamarConRetorno(%s,(%u,%u,%u));", etiqueta, donde_retornar.numeroPagina, donde_retornar.offset, donde_retornar.size);

	// Busco PC de la primera instruccion de la funcion
	int pc_first_intruction = metadata_buscar_etiqueta(etiqueta, pcb_global.indiceEtiquetas, pcb_global.tam_indiceEtiquetas);

	// Creo nuevo contexto, y guardo retPos y retVar
	list_add(pcb_global.indiceStack, stack_create(pcb_global.pc, donde_retornar.numeroPagina, donde_retornar.offset, donde_retornar.size));
	pcb_global.sp++;

	// Cambio el PC actual
	pcb_global.pc = pc_first_intruction;
	log_trace(logger, "----> PC: %u", pcb_global.pc);
}

void parser_finalizar(){
	log_trace(logger, "--> parser_finalizar();");
	log_trace(logger, "----> SP: %u", pcb_global.sp);

	t_indiceStack *aux_stack;

	// Si es el contexto principal => Finalizo programa
	if(pcb_global.sp == 0){
		estado_ejecucion = 1; // Pongo el estado de ejecucion en "Finalizado"
		return;
	}

	// Obtengo datos
	aux_stack = list_get(pcb_global.indiceStack, pcb_global.sp);

	// Actualizo PC
	pcb_global.pc = aux_stack->retPos;

	// Borro contexto
	aux_stack = list_remove(pcb_global.indiceStack, pcb_global.sp);
	pcb_global.sp--;

	// Destruyo los args
	list_destroy_and_destroy_elements(aux_stack->args, (void*) args_destroy);
	// Destruyo los vars
	list_destroy_and_destroy_elements(aux_stack->vars, (void*) vars_destroy);
	// Destruyo el contexto
	stack_destroy(aux_stack);
}

void parser_retornar(t_valor_variable retorno){
	log_trace(logger, "--> parser_retornar(%i);", retorno);

	t_indiceStack *aux_stack;

	// Obtengo datos
	aux_stack = list_get(pcb_global.indiceStack, pcb_global.sp);

	// Actualizo PC
	pcb_global.pc = aux_stack->retPos;

	//
	t_puntero aux_puntero = posToPuntero(aux_stack->retVar);

	// Asigno lo devuelto en la varaible correspondiente
	parser_asignar(aux_puntero, retorno);

	// Borro contexto
	aux_stack = list_remove(pcb_global.indiceStack, pcb_global.sp);
	pcb_global.sp--;

	// Destruyo los args
	list_destroy_and_destroy_elements(aux_stack->args, (void*) args_destroy);
	// Destruyo los vars
	list_destroy_and_destroy_elements(aux_stack->vars, (void*) vars_destroy);
	// Destruyo el contexto
	stack_destroy(aux_stack);
}

void parser_imprimir(t_valor_variable valor_mostrar){
	log_trace(logger, "--> parser_imprimir(%i);", valor_mostrar);

	// Variables
	t_mensaje mensaje;
	unsigned parametros[2];
	parametros[0] = pcb_global.pid;	// PID
	parametros[1] = valor_mostrar;	// Valor a imprimir
	mensaje.head.codigo = IMPRIMIR_NUM;
	mensaje.head.cantidad_parametros = 2;
	mensaje.head.tam_extra = 0;
	mensaje.parametros = parametros;
	mensaje.mensaje_extra = NULL;

	// Envio al UMC la peticion
	enviarMensajeNucleo(mensaje);
}

void parser_imprimirTexto(char* texto){
	log_trace(logger, "--> parser_imprimirTexto(%s);", texto);

	// Variables
	t_mensaje mensaje;
	unsigned parametros[1];
	parametros[0] = pcb_global.pid;	// PID
	mensaje.head.codigo = IMPRIMIR_TEXTO;
	mensaje.head.cantidad_parametros = 1;
	mensaje.head.tam_extra = strlen(texto) + 1;
	mensaje.parametros = parametros;
	mensaje.mensaje_extra = texto;
	memset(mensaje.mensaje_extra + mensaje.head.tam_extra - 1, '\0', 1);

	// Envio al UMC la peticion
	enviarMensajeNucleo(mensaje);
}

void parser_entradaSalida(t_nombre_dispositivo dispositivo, int tiempo){
	log_trace(logger, "--> parser_entradaSalida(%s, %i);", dispositivo, tiempo);

	// Variables
	t_mensaje mensaje;
	unsigned parametros[1];
	parametros[0] = tiempo;
	mensaje.head.codigo = ENTRADA_SALIDA;
	mensaje.head.cantidad_parametros = 1;
	mensaje.head.tam_extra = strlen(dispositivo) + 1;
	mensaje.parametros = parametros;
	mensaje.mensaje_extra = dispositivo;

	// Envio al UMC la peticion
	enviarMensajeNucleo(mensaje);

	estado_ejecucion = 3;
}
void parser_wait(t_nombre_semaforo identificador_semaforo){
	log_trace(logger, "--> parser_wait(%s);", identificador_semaforo);

	// Variables
	t_mensaje mensaje;
	unsigned parametros[1];
	parametros[0] = pcb_global.pid;	// PID
	mensaje.head.codigo = WAIT;
	mensaje.head.cantidad_parametros = 1;
	mensaje.head.tam_extra = strlen(identificador_semaforo) + 1;
	mensaje.parametros = parametros;
	mensaje.mensaje_extra = identificador_semaforo;

	// Envio al UMC la peticion
	enviarMensajeNucleo(mensaje);

	// Recibo mensaje
	recibirMensajeNucleo(&mensaje);

	if(mensaje.head.codigo != CPU_WAIT){
		log_error(logger, "Se recibio un mensaje diferente a 'CPU_WAIT'");
		abort();
	}

	if(mensaje.parametros[0] == 1){
		estado_ejecucion = 4;
	}

	// Libero memoria de mensaje
	freeMensaje(&mensaje);

}

void parser_signal(t_nombre_semaforo identificador_semaforo){
	log_trace(logger, "parser_signal(%s);", identificador_semaforo);

	// Variables
	t_mensaje mensaje;
	unsigned parametros[1];
	parametros[0] = pcb_global.pid;	// PID
	mensaje.head.codigo = SIGNAL;
	mensaje.head.cantidad_parametros = 1;
	mensaje.head.tam_extra = strlen(identificador_semaforo) + 1;
	mensaje.parametros = parametros;
	mensaje.mensaje_extra = identificador_semaforo;

	// Envio al UMC la peticion
	enviarMensajeNucleo(mensaje);

	// Libero memoria de mensaje
	free(mensaje.mensaje_extra);
}

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
