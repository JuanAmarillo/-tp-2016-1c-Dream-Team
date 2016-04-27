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
#include "cpu.h"

#define IP "127.0.0.1"
#define PUERTO "6667"
#define PACKAGESIZE 1024

int main(int argc, char** argv){

	// Leer archivo config.conf
	leerArchivoConfig();

	// Me conecto a la UMC
	socketUMC = conectarseUMC();
	if(socketUMC == -1) abort();

	// Me conecto a el Nucleo
	//socketNucleo = conectarseNucleo();
	//if(socketNucleo == -1) abort();

	int enviar = 1;
	char message[PACKAGESIZE];
	printf("Conectado al servidor. Bienvenido al sistema, ya puede enviar mensajes. Escriba 'exit' para salir\n");
	while(enviar){
		fgets(message, PACKAGESIZE, stdin);
		if (!strcmp(message,"exit\n")) enviar = 0;
		if (enviar) send(socketUMC, message, strlen(message) + 1, 0);
	}
	close(socketUMC);


	testParser();


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
 * Descripcion: Procedimiento que establece conexion con la UMC
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
 * Descripcion: Procedimiento que establece conexion con el Nucleo
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
 * crearConexion(const char *ip, int puerto);
 * Parametros: ip, puerto
 * Descripcion: Procedimiento que establece conexion con el Nucleo
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
