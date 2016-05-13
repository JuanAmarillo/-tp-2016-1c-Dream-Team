/*
 * swap.c
 *
 *  Created on: 26/4/2016
 *      Author: utnso
 */

#include "swap.h"


int main(){
	readConfigFile();
	crearArchivoSWAP();
	configArchivoSWAP();
	setSocket();
	bindSocket();
	acceptSocket();
	return 0;
}

void readConfigFile(){
	t_config *config = config_create("config.conf");
		if (config == NULL) {
			free(config);
			abort();
		}
	PUERTO_ESCUCHA = atoi(config_get_string_value(config, "PUERTO_ESCUCHA"));
	NOMBRE_SWAP = config_get_string_value(config, "NOMBRE_SWAP");
	CANTIDAD_PAGINAS= atoi(config_get_string_value(config, "CANTIDAD_PAGINAS"));
	TAMANIO_PAGINA= atoi(config_get_string_value(config, "TAMANIO_PAGINA"));
	RETARDO_COMPACTACION= atoi(config_get_string_value(config, "RETARDO_COMPACTACION"));
}

void crearArchivoSWAP(){
	char* comandoCreacion;
	strcpy(comandoCreacion, "dd of=%s bs=%d count=%d", NOMBRE_SWAP, TAMANIO_PAGINA, CANTIDAD_PAGINAS);
	if (system(comandoCreacion)){
		printf("No se pudo crear el archivo %s", NOMBRE_SWAP);
		exit(1);
	}
}

void configArchivoSWAP(){
	SWAPFILE= fopen(NOMBRE_SWAP, "r+");
	fwrite('\0', 1, (CANTIDAD_PAGINAS*TAMANIO_PAGINA),SWAPFILE);
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
	int status = 1;
	char mensaje[MSG_SIZE];
	struct sockaddr_in addr;	// Esta estructura contendra los datos de la conexion del cliente. IP, puerto, etc.
	socklen_t addrlen = sizeof(addr);
	listen(listeningSocket,10);
	int socketCliente = accept(listeningSocket, (struct sockaddr *) &addr, &addrlen);
	printf("Se ha conectado al UMC\n");
}
