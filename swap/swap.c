/*
 * swap.c
 *
 *  Created on: 26/4/2016
 *      Author: utnso
 */
#include <commons/config.h>
#include <commons/string.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include "swap.h"
#define MSG_SIZE 50+1

void escuchar() {
	int status = 1;
	char mensaje[MSG_SIZE];
	listen(listeningSocket,10);
	struct sockaddr_in addr;			// Esta estructura contendra los datos de la conexion del cliente. IP, puerto, etc.
	socklen_t addrlen = sizeof(addr);
	int socketCliente = accept(listeningSocket, (struct sockaddr *) &addr, &addrlen);
	printf("UMC conectada al Administrador SWAP\n");
	printf("Mensajes Recibidos:\n");
	while (status != 0) {
		status = recv(socketCliente, (void*) mensaje, MSG_SIZE,0);
		if(status!=0) printf("%s", mensaje);
	}
}

int main(){
	startUp();
	setUMCAdress();
	//setSocketsAsClient(); Inutilizado por la primera entrega
	setSocketsAsServer();
	escuchar();
	return 0;
}

void startUp(){
	memset(&hintsServ, 0, sizeof(hintsServ));
		hintsServ.ai_family = AF_UNSPEC;
		hintsServ.ai_flags = AI_PASSIVE;
		hintsServ.ai_socktype = SOCK_STREAM;
	memset(&hintsClient, 0, sizeof(hintsClient));
		hintsClient.ai_family = AF_UNSPEC;
		hintsClient.ai_socktype = SOCK_STREAM;
}

void setUMCAdress(){
	t_config *config = config_create("config.conf");
		if (config == NULL) {
			free(config);
			abort();
		}
	infoConfig.ip_umc = config_get_string_value(config, "IP_UMC");
	infoConfig.port_umc = config_get_string_value(config, "IP_UMC");
	infoConfig.self_port = config_get_string_value(config, "PORT");

}


void setSocketsAsClient(){
	getaddrinfo(infoConfig.ip_umc,infoConfig.port_umc,&hintsClient,&infoAsClient);
	mi_fd=socket(infoAsClient->ai_family,infoAsClient->ai_socktype,infoAsClient->ai_protocol);
	connect(mi_fd, infoAsClient->ai_addr,infoAsClient->ai_addrlen);
	freeaddrinfo(infoAsClient);
}

void setSocketsAsServer(){
	getaddrinfo(NULL,infoConfig.self_port,&hintsClient,&infoAsServer);
	listeningSocket=socket(infoAsServer->ai_family,infoAsServer->ai_socktype, infoAsServer->ai_protocol);
	bind(listeningSocket,infoAsServer->ai_addr,infoAsServer->ai_addrlen);
	freeaddrinfo(infoAsServer);
}

