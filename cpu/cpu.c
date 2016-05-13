#include <commons/config.h>
#include <commons/collections/list.h>
#include <commons/collections/dictionary.h>
#include <commons/string.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <parser/parser.h>
#include "analizador.h"
#include "protocolo_mensaje.h"
#include "cpu.h"

int main(int argc, char** argv){

	// Leer archivo config.conf
	leerArchivoConfig();
	testMensajeProtocolo();

	/*

	// Me conecto a la UMC
	socketUMC = conectarseUMC();
	if(socketUMC == -1) abort();

	// Me conecto a el Nucleo
	socketNucleo = conectarseNucleo();
	if(socketNucleo == -1) abort();

	//testParser();

	*/

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
int enviarMensajeUMC(mensaje_t mensaje) {
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
int enviarMensajeNucleo(mensaje_t mensaje) {
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
int recibirMensajeNucleo(mensaje_t *mensaje) {
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
int recibirMensajeUMC(mensaje_t *mensaje) {
	int recibir = recibirMensaje(socketUMC,mensaje);
	if(recibir == -1){
		perror("Error al recibir mensaje de la UMC");
		return -1;
	}
	return recibir;
}

/*
 * testParser();
 * Parametros: -
 * Descripcion: Procedimiento que simula ejecutar un codigo ansisop
 * Return: -
 */
void testParser() {
	// Definir variable
	printf("Ejecutando '%s'\n", DEFINICION_VARIABLES);
	analizadorLinea(strdup(DEFINICION_VARIABLES), &functions, &kernel_functions);
	printf("================\n");
	// Asignar
	printf("Ejecutando '%s'\n", ASIGNACION);
	analizadorLinea(strdup(ASIGNACION), &functions, &kernel_functions);
	printf("================\n");
	// Imprimir
	printf("Ejecutando '%s'\n", IMPRIMIR);
	analizadorLinea(strdup(IMPRIMIR), &functions, &kernel_functions);
	printf("================\n");
	// Imprimir texto
	printf("Ejecutando '%s'", IMPRIMIR_TEXTO);
	analizadorLinea(strdup(IMPRIMIR_TEXTO), &functions, &kernel_functions);
	printf("================\n");
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
 * FUNCIONES ANALIZADOR
 */

t_puntero definirVariable(t_nombre_variable variable) {
	printf("definir la variable %c\n", variable);
	return POSICION_MEMORIA;
}

t_puntero obtenerPosicionVariable(t_nombre_variable variable) {
	printf("Obtener posicion de %c\n", variable);
	return POSICION_MEMORIA;
}

t_valor_variable dereferenciar(t_puntero puntero) {
	printf("Dereferenciar %d y su valor es: %d\n", puntero, CONTENIDO_VARIABLE);
	return CONTENIDO_VARIABLE;
}

void asignar(t_puntero puntero, t_valor_variable variable) {
	printf("Asignando en %d el valor %d\n", puntero, variable);
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
