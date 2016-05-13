/*
 * swap.c
 *
 *  Created on: 26/4/2016
 *      Author: utnso
 */

#include "swap.h"


int main(){
	readConfigFile();
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
}

void setSocket(){
		myAddress.sin_family = AF_INET;
		myAddress.sin_port = htons(atoi(myPort));
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
