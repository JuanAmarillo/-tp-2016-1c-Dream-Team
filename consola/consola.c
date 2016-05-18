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
#define MSG_SIZE 50+1
struct sockaddr_in direccionNucleo;

int main(int argc, char** argv){

	// Leer archivo config.conf
	leerArchivoConfig();
	//inicializar estructura de socket con los datos del nucleo
	inicializarDireccionNucleo();
	int miSocket = socket(PF_INET, SOCK_STREAM, 0);
	if (connect (miSocket, (struct sockaddr*) &direccionNucleo, sizeof(struct sockaddr_in)) == -1) {
		printf("Error de connect\n");
		exit(1);
	}
	char mensaje[MSG_SIZE];
	printf ("Conectado al servidor. Bienvenido al sistema, ya puede enviar mensajes. Escriba 'exit' para salir\n");
	int enviar = 1;
	while (enviar) {
		//A partir de acá cambia el código?
		fgets (mensaje, MSG_SIZE, stdin);
		if (!strcmp (mensaje, "exit\n")) enviar = 0;
		if (enviar) send (miSocket, mensaje, strlen(mensaje) +1, 0);
	}
	close (miSocket);
	return EXIT_SUCCESS;
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
