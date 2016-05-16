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


typedef struct{
	int i;
}estructuraAGuardar;
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

//PROTOTIPOS DE FUNCIONES INICIALES
void readConfigFile();
void crearArchivoSWAP();
void crearEstructurasDeManejo();
void limpiarI_P(int);
//PROTOTIPO DE FUNCIONES DE SOCKETS
void setSocket();
void bindSocket();
void acceptSocket();
//PROTOTIPO DE FUNCIONES DE MANEJO DE PAGINAS
void assignPage(unsigned nroPag, estructuraAGuardar* str);
void unAssignPage(unsigned nroPag);
estructuraAGuardar* getPage(unsigned nroPag); // TENGO QUE DEFINIR QUE ESTRUCTURA VOY A UTILIZAR PARA REPRESENTAR A UNA PAGINA
/*
 *	La funcion busca los espacios disponibles en el archivo SWAP para asignar un proyecto.
 *	Si la funcion devuelve 0, o un numero positivo, este es el numero de pagina donde inicia el segmento de paginas vacias para asignar el proyecto.
 *	Si la funcion devuelve -1, se debe realizar una compactacion para asignar el proyecto.
 *	Si la funcion devuelve -2, no hay memoria disponible en el SWAP y se debe avisar a la UMC para rechazar el programa.
 **/
unsigned searchSpaceToFill(unsigned size);
//PROTOTIPO DE FUNCIONES FINALES
void accionesDeFinalizacion();


#endif /* SWAP_H_ */
