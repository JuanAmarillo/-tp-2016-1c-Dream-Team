#include <commons/config.h>
#include <commons/string.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include "consola.h"
#include "protocolo_mensaje.h"
#include "../messageCode/messageCode.h"

int main(int argc, char** argv) {

	t_mensaje mensaje;

	// Verifica cantidad de parámetros
	if (argc>2) {
		printf ("Error. Hay más de dos argumentos.\n");
		return -2;
	}

	if (argc==1) {
		printf ("Error. Falta un parámetro.\n");
		return -3;
	}

	// Abre el archivo en modo lectura
	FILE * file;
	file = fopen( argv[1] , "r");


	// Verifica si se puede abrir
	if (file==NULL){
		printf ("No se puede abrir el archivo. \n");
		return -4;
	}

	// Me posiciono al final del archivo
	fseek (file, 0, SEEK_END);

	// Obtengo el tamaño
	long int tamanio = ftell(file);

	// Me posiciono al principio del archivo
	fseek (file, 0, SEEK_SET);

	// Asumo que malloc cas
	char* codigo = malloc (tamanio+1);

	// Se creo el bloque en memoria ? Suele no fallar, pero por si ponen tamaños de codigos grandes
	if(codigo == NULL){
		printf("No se inicializo el malloc \n");
		return 0;
	}
	// Copio el codigo
	fread(codigo,tamanio,1,file);

	// Agrego \0 al final por las dudas
	memset(codigo+tamanio,'\0',1);

	// Cierro archivo
	fclose(file);

	// Leer archivo config.conf
	leerArchivoConfig();

	// Muestro codigo por pantalla
	printf("Codigo AnsiSOP:\n'%s'\n\n", codigo);

	//Me conecto con el nucleo
	int socketNucleo = crearConexion(infoConfig.ip, infoConfig.puerto);
	if (socketNucleo <= 0) {
		printf("Error de connect\n");
		return 0;
	}

	mensaje = codigo_to_mensaje(codigo);

	if(enviarMensaje(socketNucleo, mensaje) <= 0){
		perror("Error al enviar Programa\n");
		exit(1);
	}

	free(codigo);
	free(mensaje.mensaje_extra);
	
	t_mensaje mensaje_recibido;

	while(1){
		
		if (recibirMensaje(socketNucleo,&mensaje_recibido) <= 0){
			perror("Se desconecto el nucleo\n");
			return 0;
		}
	
		switch(mensaje_recibido.head.codigo){
			case IMPRIMIR_PROGRAMA:
				printf("Imprimir: %d \n", mensaje_recibido.parametros[0]);
				break;
			case IMPRIMIR_TEXTO_PROGRAMA:
				printf("Imprimir texto: %s \n", mensaje_recibido.mensaje_extra);
				break;
			case EXIT_PROGRAMA:
				printf("Finalizo el programa. \n");
				return 0;
			case ERROR_PROGRAMA:
				perror(mensaje_recibido.mensaje_extra);
				exit(1);
				break;
			default:
				printf("Codigo invalido \n");
				break;
		}

		freeMensaje(&mensaje_recibido);
	}
	return 0;
}



void leerArchivoConfig() {

	t_config *config = config_create("config.conf");

	if (config == NULL) {
		free(config);
		abort();
	}

	// Guardo los datos en una variable global
	infoConfig.ip = config_get_string_value(config, "IP");
	infoConfig.puerto = config_get_string_value(config, "PUERTO");

	// No uso config_destroy(config) porque bugea
	free(config->path);
	free(config);
}

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

t_mensaje codigo_to_mensaje(char* codigo){
	unsigned int tamCod = strlen(codigo)+1;
	t_mensajeHead head = {NUEVO_PROGRAMA, 0, tamCod};
	t_mensaje mensaje;
	mensaje.head = head;
	mensaje.parametros = NULL;
	mensaje.mensaje_extra = malloc(tamCod);
	memcpy(mensaje.mensaje_extra, codigo, tamCod);
	memset(mensaje.mensaje_extra + tamCod - 1, '\0', 1);
	return mensaje;
}
