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
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
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
int servidorUMC,clienteSWAP,clienteUMC;
struct sockaddr_in direccionServidorUMC;
struct sockaddr_in direccionServidorSWAP;
fd_set master;
char* buffer; // probablemente lo termine sacando

/*
 * Funciones
 */
struct sockaddr_in setDireccion(const char *puerto);
void gestionarConexiones();
void leerArchivoConfig();
int recibirConexiones();
void aceptarConexion();
int recibirDatos();
void conectarAlSWAP();
void enviarDatos();



#endif /* UMC_H_ */
