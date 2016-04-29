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
int listeningSocket;
struct sockaddr_in addressForUMC;

//PROTOTIPOS DE FUNCIONES
void startUp();
void readConfigFile();
void setSocketsAsClient();
void setSocketsAsServer();
void escuchar();
#endif /* SWAP_H_ */
