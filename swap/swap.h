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

//VARIABLES DEL ARCHIVO DE CONFIGURACION
int PUERTO_ESCUCHA;
char* NOMBRE_SWAP;
int CANTIDAD_PAGINAS;
int TAMANIO_PAGINA;
int RETARDO_COMPACTACION;


//VARIABLES GLOBALES
int listeningSocket;
struct sockaddr_in myAddress;
FILE* SWAPFILE;

//PROTOTIPOS DE FUNCIONES
void readConfigFile();
void crearArchivoSWAP();
void setSocket();
void bindSocket();
void acceptSocket();
void configArchivoSWAP();


#endif /* SWAP_H_ */
