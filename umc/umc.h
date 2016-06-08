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
#include <pthread.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <commons/config.h>
#include <commons/string.h>
#include <commons/collections/list.h>
#include "messageCode.h"

/*
 * Estructuras de datos
 */
typedef struct{
	char *ip;
	char *puertoUMC;
	char *puertoSWAP;
} t_infoConfig;

typedef struct{
	int marcos;
	int tamanioDeMarcos;
	int maxMarcosPorPrograma;
} t_memoria;

typedef struct {
  unsigned codigo;
  unsigned cantidad_parametros;
  unsigned tam_extra;
} t_mensajeHead;

typedef struct {
  t_mensajeHead head;
  unsigned *parametros;
  char *mensaje_extra;
} t_mensaje;

typedef struct{
	unsigned estaEnMemoria;
	unsigned fueModificado;
	unsigned marco;
} t_entradaTablaPaginas;

typedef struct{
	unsigned pid;
	t_entradaTablaPaginas *entradaTablaPaginas;
} t_tablaDePaginas;





/*
 * Variables Globales
 */
t_memoria infoMemoria;
t_infoConfig infoConfig;
int servidorUMC,clienteSWAP;
struct sockaddr_in direccionServidorUMC;
struct sockaddr_in direccionServidorSWAP;
fd_set master;
unsigned procesoActivo;
void* memoriaPrincipal;
t_list *tablasDePaginas;
pthread_mutex_t mutex;

/*
 * Funciones
 */
struct sockaddr_in setDireccion(const char *puerto);
void gestionarConexiones();
void leerArchivoConfig();
int recibirConexiones();
int aceptarConexion();
//int recibirDatos();
void conectarAlSWAP();
void enviarDatos();



#endif /* UMC_H_ */
