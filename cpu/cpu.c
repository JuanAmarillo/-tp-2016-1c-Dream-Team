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

#define PACKAGESIZE 1024

int main(int argc, char** argv){

	// Leer archivo config.conf
	leerArchivoConfig();

	// Me conecto a la UMC
	//socketUMC = conectarseUMC();
	//if(socketUMC == -1) abort();

	// Me conecto a el Nucleo
	socketNucleo = conectarseNucleo();
	if(socketNucleo == -1) abort();

	// TEST CLIENTE PARA NUCLEO
	int enviar = 1;
	char message[PACKAGESIZE];
	printf("Conectado al servidor. Bienvenido al sistema, ya puede enviar mensajes. Escriba 'exit' para salir\n");
	while(enviar){
		fgets(message, PACKAGESIZE, stdin);
		if (!strcmp(message,"exit\n")) enviar = 0;
		if (enviar) send(socketNucleo, message, strlen(message) + 1, 0);
	}
	close(socketNucleo);


	testParser();


	return EXIT_SUCCESS;
}
