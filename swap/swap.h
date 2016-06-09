/*
 * swap.h
 *
 *  Created on: 26/4/2016
 *      Author: utnso
 */

#ifndef SWAP_H_
#define SWAP_H_

#include <commons/collections/list.h>

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
char* paginaMultiProposito;


//ACCIONES DEL SWAP
void setPage(unsigned);
void unSetPage(unsigned);
void getPage();
void savePage(unsigned);
int recibirCabecera();
void overWritePage(int);
void saveProgram();
void returnPage();
void endProgram();
void saveNewPage();
void replacePages(int longitudPrograma, int inicioProg,int inicioEspacioBlanco);
void new_Or_Replace_t_infoProg(int pid, int longitudPrograma, int inicioProg,int eliminar);
void asignarEspacio(unsigned,int,unsigned);
void reservarEspacio();
void moveProgram(int, int);
void deleteEmptySpaces();


#endif /* SWAP_H_ */
