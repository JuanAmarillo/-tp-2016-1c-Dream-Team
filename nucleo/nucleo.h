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

/*
 * Estructuras de datos
 */

/*Variables Globales*/
/*----------------------------------------------------------*/

/*
 * Funciones / Procedimientos
 */
void leerArchivoConfig(void);// Lee archivo config.conf
void inicializarDirecciones(void);//Inicializa las direcciones propias para escuchar y la dirección de la UMC
void conectar_a_umc(void);//Conectarse al proceso UMC
void abrirPuertos(void);//Pone en modo escucha los sockets asociados a los puertos para Consola y para CPU
int maximofd(int, int);//Compara valor numérico entre dos file descriptors y devuelve el máximo
void administrarConexiones(void);//Contiene todos los procedimientos para recibir datos de los otros procesos

#endif /* NUCLEO_H_ */

