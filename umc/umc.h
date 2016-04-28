/*
 * umc.h
 *
 *  Created on: 27/4/2016
 *      Author: utnso
 */

#ifndef UMC_H_
#define UMC_H_

/*
 * Bibliotecas
 */
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <commons/config.h>
#include <commons/string.h>

/*
 * Estructuras de datos
 */
typedef struct{
	char *ip;
	char *puertoUMC;
	char *puertoSWAP;
} t_infoConfig;

/*
 * Variables Globales
 */
t_infoConfig infoConfig;
int servidorUMC,clienteCPU;
struct sockaddr_in direccionServidorUMC;
struct sockaddr_in direccionServidorSWAP;

#endif /* UMC_H_ */
