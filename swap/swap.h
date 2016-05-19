/*
 * swap.h
 *
 *  Created on: 26/4/2016
 *      Author: utnso
 */

#ifndef SWAP_H_
#define SWAP_H_

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
int socketCliente;

//VARIABLES DE USO DEL SWAP
FILE* SWAPFILE;
t_bitarray* DISP_PAGINAS;
t_list * INFO_PROG;
char* paginaMultiProposito;

//PROTOTIPO DE FUNCIONES DE MANEJO DE PAGINAS
void setNewPage(unsigned, char*);
void unSetPage(unsigned);
char* getPage(unsigned);

/*
 *	La funcion busca los espacios disponibles en el archivo SWAP para asignar un proyecto.
 *	Si la funcion devuelve 0, o un numero positivo, este es el numero de pagina donde inicia el segmento de paginas vacias para asignar el proyecto.
 *	Si la funcion devuelve -1, se debe realizar una compactacion para asignar el proyecto.
 *	Si la funcion devuelve -2, no hay memoria disponible en el SWAP y se debe avisar a la UMC para rechazar el programa.
 **/

//ACCIONES DEL SWAP
int recibirCabecera();
void reservarEspacio();
void setPage(unsigned);
void endProgram();
void overWritePage(int);
void unSetPage(unsigned);
void saveProgram();
void savePage(unsigned);
void saveNewPage();
void endProgram();
void returnPage();
void asignarEspacio(unsigned,int,unsigned);
void compactar();
void moveProgram(int, int);

#endif /* SWAP_H_ */
