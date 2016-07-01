/*
 * initialize.c
 *
 *  Created on: 18/5/2016
 *      Author: utnso
 */
#include <commons/config.h>
#include <commons/string.h>
#include <commons/bitarray.h>
#include <commons/log.h>
#include <commons/collections/list.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include "swap.h"
#include "funcionesAuxiliares.h"
#include "initialize.h"

void readConfigFile(){
	system("clear");
	logger = log_create("SWAP.txt", "SWAP", 1, LOG_LEVEL_TRACE);
	t_config *config = config_create("config.conf");
		if (config == NULL) {
			free(config);
			abort();
		}
	PUERTO_ESCUCHA = atoi(config_get_string_value(config, "PUERTO_ESCUCHA"));
	NOMBRE_SWAP = config_get_string_value(config, "NOMBRE_SWAP");
	CANTIDAD_PAGINAS= atoi(config_get_string_value(config, "CANTIDAD_PAGINAS"));
	TAMANIO_PAGINA= atoi(config_get_string_value(config, "TAMANIO_PAGINA"));
	RETARDO_ACCESO= atoi(config_get_string_value(config, "RETARDO_ACCESO"));
	RETARDO_COMPACTACION= atoi(config_get_string_value(config, "RETARDO_COMPACTACION"));
	log_trace(logger,"Se leyo el archivo de configuracion");
}

void crearArchivoSWAP(){
	char* comandoCreacion = malloc(100);

	sprintf(comandoCreacion, "dd if=/dev/zero of=%s bs=%i count=%i", NOMBRE_SWAP, TAMANIO_PAGINA, CANTIDAD_PAGINAS);

	if (system(comandoCreacion)){
		log_error(logger,"No se pudo crear el archivo");
		exit(1);
	}
	SWAPFILE= fopen(NOMBRE_SWAP, "r+");
	log_trace(logger, "Se creo el archivo SWAP\0");
	free(comandoCreacion);
}

void crearEstructurasDeManejo(){
	int tamanio = (CANTIDAD_PAGINAS/8)+1;
	char *data = malloc(tamanio);
	strcpy(data,"\0");
	DISP_PAGINAS = bitarray_create(data,tamanio);
	INFO_PROG = list_create();
	bufferPagina = malloc(TAMANIO_PAGINA);
	*bufferPagina = "\0";

}


void setSocket(){
		myAddress.sin_family = AF_INET;
		myAddress.sin_port = htons(PUERTO_ESCUCHA);
		myAddress.sin_addr.s_addr = INADDR_ANY;
		memset(myAddress.sin_zero, '\0', sizeof(myAddress.sin_zero));
}

void bindSocket(){
	int auxiliar=1;
	listeningSocket=socket(AF_INET, SOCK_STREAM, 0);
	setsockopt(listeningSocket, SOL_SOCKET, SO_REUSEADDR, &auxiliar, sizeof(int));
	bind(listeningSocket,(struct sockaddr*) &myAddress, sizeof(struct sockaddr));
}

void acceptSocket() {
	struct sockaddr_in addr;	// Esta estructura contendra los datos de la conexion del cliente. IP, puerto, etc.
	socklen_t addrlen = sizeof(addr);
	listen(listeningSocket,10);
	socketCliente = accept(listeningSocket, (struct sockaddr *) &addr, &addrlen);
	printf("Se ha conectado al UMC\n");
}

void accionesDeFinalizacion() {
	fclose(SWAPFILE);
	bitarray_destroy(DISP_PAGINAS);
	free(bufferPagina);
	list_destroy_and_destroy_elements(INFO_PROG,(void*) infoProg_destroy);
}
