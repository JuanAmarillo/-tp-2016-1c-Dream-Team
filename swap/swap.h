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

//VARIABLES DE SOCKETS
int listeningSocket;
struct sockaddr_in myAddress;

//VARIABLES DE USO DEL SWAP
FILE* SWAPFILE;
t_bitarray* DISP_PAGINAS;
t_infoProg* INFO_PROG;

//PROTOTIPOS DE FUNCIONES
void readConfigFile();
void crearArchivoSWAP();
void crearEstructurasDeManejo();
void limpiarI_P(int);
void setSocket();
void bindSocket();
void acceptSocket();
void accionesDeFinalizacion();


#endif /* SWAP_H_ */
