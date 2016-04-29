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
	char *puertoNucleo;
} t_infoConfig;

/*
 * Variables Globales
 */
t_infoConfig infoConfig;
int servidorUMC,clienteSWAP,clienteNucleo,clienteCPU;
struct sockaddr_in direccionServidorUMC;
struct sockaddr_in direccionServidorSWAP;
char* buffer; // problablemente lo termine sacando

/*
 * Funciones
 */
struct sockaddr_in setDireccion(const char *puerto);
void leerArchivoConfig();
void recibirConexiones();
void aceptarConexion();
void recibirDatos();
void conectarAlSWAP();
void enviarDatos();



#endif /* UMC_H_ */
