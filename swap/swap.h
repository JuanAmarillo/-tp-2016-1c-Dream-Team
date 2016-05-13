/*
 * swap.h
 *
 *  Created on: 26/4/2016
 *      Author: utnso
 */

#ifndef SWAP_H_
#define SWAP_H_

#define MSG_SIZE 50+1
#define myPort 5004
#include <commons/config.h>
#include <commons/string.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>

typedef struct{
	char *ip_umc;
	char *port_umc;
} t_infoConfig;

//VARIABLES GLOBALES
t_infoConfig infoConfig;
int listeningSocket;
struct sockaddr_in myAddress;

//PROTOTIPOS DE FUNCIONES
void readConfigFile();
void setSocket();
void bindSocket();
void acceptSocket();


#endif /* SWAP_H_ */
