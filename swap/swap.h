/*
 * swap.h
 *
 *  Created on: 26/4/2016
 *      Author: utnso
 */

#ifndef SWAP_H_
#define SWAP_H_

typedef struct{
	char *ip_umc;
	char *port_umc;
	char *self_port;
} t_infoConfig;

//VARIABLES GLOBALES
t_infoConfig infoConfig;
struct addrinfo hintsClient, hintsServ, *infoAsClient, *infoAsServer;
int mi_fd;
int listeningSocket;
int SELFPORT;


//PROTOTIPOS DE FUNCIONES
void startUp();
void setUMCAdress();
void setSocketsAsClient();
void setSocketsAsServer();

#endif /* SWAP_H_ */
