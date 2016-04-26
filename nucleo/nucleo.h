/*
 * nucleo.h
 *
 *  Created on: 17/4/2016
 *      Author: utnso
 */

#ifndef NUCLEO_H_
#define NUCLEO_H_

//Bibliotecas a usar
#include <commons/config.h>
#include <commons/string.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/time.h>
#include <unistd.h>

//Cuántas conexiones se aceptarán
#define BACKLOG 100
//Puerto de el propio núcleo
#define PUERTO 9090
/*
 * Estructuras de datos
 */
typedef struct{
	char *puerto_prog;
	char *puerto_cpu;
	char *puerto_umc;
} t_infoConfig;

/*
 * Variables Globales
 */
t_infoConfig infoConfig;


/*
 * Funciones / Procedimientos
 */
void leerArchivoConfig();

#endif /* NUCLEO_H_ */
