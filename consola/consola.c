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
#define MSG_SIZE 50+1

struct sockaddr_in direccionNucleo;

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

	// Comprueba el tamaño del archivo
	fseek (file, 0, SEEK_END);

	// Reserva lugar para lo que va a copiar
	long int tamanio = ftell(file);

	fseek (file, 0, SEEK_SET);

	char* codigo = malloc (tamanio+1); //Chequear que sea posible reservar memoria
	fread(codigo,tamanio,1,file);
	fclose(file);

	// Leer archivo config.conf
	leerArchivoConfig();

	printf("Codigo AnsiSOP:\n%s\n\n", codigo);

	//inicializar estructura de socket con los datos del nucleo
	inicializarDireccionNucleo();
	int miSocket = socket(PF_INET, SOCK_STREAM, 0);
	if (connect (miSocket, (struct sockaddr*) &direccionNucleo, sizeof(struct sockaddr_in)) == -1) {
		printf("Error de connect\n");
		exit(1);
		}
	printf ("Conectado al servidor. Bienvenido al sistema, ya puede enviar mensajes. Escriba 'exit' para salir\n");

	mensaje = codigo_to_mensaje(codigo);

	if(enviarMensaje(miSocket, mensaje) == -1)
	{
		perror("Error al enviar Programa\n");
		exit(1);
	}

	free(codigo);
	free(mensaje.mensaje_extra);
	
	t_mensaje mensaje_recibido;
	if (recibirMensaje(miSocket,&mensaje_recibido)==-1){
		perror("Error al recibir mensaje\n");
		exit(1);
	}

	switch(mensaje_recibido.head.codigo){
		case IMPRIMIR_PROGRAMA:
			printf("Imprimir: %d \n", mensaje_recibido.head.codigo);
			break;
		case IMPRIMIR_TEXTO_PROGAMA:
			printf("Imprimir texto: %c \n", mensaje_recibido.head.codigo);
			break;
		default:
			printf("Codigo invalido \n");
			break;
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



void inicializarDireccionNucleo (){
	direccionNucleo.sin_family = AF_INET;
	direccionNucleo.sin_port = htons (atoi(infoConfig.puerto));
	direccionNucleo.sin_addr.s_addr = inet_addr(infoConfig.ip);
	memset (direccionNucleo.sin_zero, '\0', sizeof (direccionNucleo.sin_zero));
}
