/*
 * swap.h
 *
 *  Created on: 26/4/2016
 *      Author: utnso
 */

#ifndef SWAP_H_
#define SWAP_H_

//#include "protocolo_mensaje.h"
#include <commons/collections/list.h>
#include <commons/bitarray.h>
#include <commons/log.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include "protocolo_mensaje.h"
//DECLARACION DE ESTRUCTURAS
typedef struct{
	unsigned PID;
	unsigned PAG_INICIAL;
	unsigned LONGITUD;
}t_infoProg;

//VARIABLES DEL ARCHIVO DE CONFIGURACION
int PUERTO_ESCUCHA;
char* NOMBRE_SWAP;
int CANTIDAD_PAGINAS;
int TAMANIO_PAGINA;
int RETARDO_COMPACTACION;
int RETARDO_ACCESO;

//VARIABLES DE SOCKETS
int listeningSocket;
struct sockaddr_in myAddress;
int socketCliente;

//VARIABLES DE USO DEL SWAP
FILE* SWAPFILE;
t_bitarray* DISP_PAGINAS;
t_list *INFO_PROG;
char* bufferPagina;
t_mensaje received;
t_log *logger;


//ACCIONES DEL SWAP
void socketConf();
void initialConf();
int funcionamientoSWAP();
void setPage(unsigned);
void unSetPage(unsigned);
void getPage(unsigned);
void savePage(unsigned);
int recibirCabecera();
void saveProgram();
void returnPage();
void endProgram();
void saveNewPage();
void replacePages(int longitudPrograma, int inicioProg,int inicioEspacioBlanco);
void agregarAlINFOPROG(t_infoProg* new);
void new_Or_Replace_t_infoProg(int pid, int longitudPrograma, int inicioProg,int eliminar);
void asignarEspacio(unsigned,int,unsigned);
void reservarEspacio();
void moveProgram(int, int);
void deleteEmptySpaces();


#endif /* SWAP_H_ */
