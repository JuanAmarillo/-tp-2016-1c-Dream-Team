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

int main(){
	readConfigFile();
	startUp();
	//setSocketsAsClient(); Inutilizado por la primera entrega
	setSocketsAsServer();
	escuchar();
	return 0;
}

void startUp(){
		addressForUMC.sin_family = AF_INET;
		puts("Holi");
		addressForUMC.sin_port = htons(atoi(infoConfig.port_umc));
		puts("Holi");
		addressForUMC.sin_addr.s_addr = INADDR_ANY;
		memset(addressForUMC.sin_zero, '\0', sizeof(addressForUMC.sin_zero));
}

void readConfigFile(){
	t_config *config = config_create("config.conf");
		if (config == NULL) {
			free(config);
			abort();
		}
	infoConfig.ip_umc = config_get_string_value(config, "IP_UMC");
	infoConfig.port_umc = config_get_string_value(config, "PORT_UMC");
}

/*
void setSocketsAsClient(){
	getaddrinfo(infoConfig.ip_umc,infoConfig.port_umc,&hintsClient,&infoAsClient);
	mi_fd=socket(infoAsClient->ai_family,infoAsClient->ai_socktype,infoAsClient->ai_protocol);
	connect(mi_fd, infoAsClient->ai_addr,infoAsClient->ai_addrlen);
	freeaddrinfo(infoAsClient);
}
*/

void setSocketsAsServer(){
	int auxiliar=1;
	listeningSocket=socket(AF_INET, SOCK_STREAM, 0);
	setsockopt(listeningSocket, SOL_SOCKET, SO_REUSEADDR, &auxiliar, sizeof(int));
	bind(listeningSocket,(struct sockaddr*) &addressForUMC, sizeof(struct sockaddr));
}

void escuchar() {
	int status = 1;
	char mensaje[MSG_SIZE];
	struct sockaddr_in addr;	// Esta estructura contendra los datos de la conexion del cliente. IP, puerto, etc.
	socklen_t addrlen = sizeof(addr);
	listen(listeningSocket,10);
	int socketCliente = accept(listeningSocket, (struct sockaddr *) &addr, &addrlen);
	printf("UMC conectada al Administrador SWAP\n");
	printf("Mensajes Recibidos:\n");
	while (status != 0) {
		status = recv(socketCliente, (void*) mensaje, MSG_SIZE,0);
		if(status!=0) printf("%s", mensaje);
	}
}
