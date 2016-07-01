/*
 * funcionesAuxiliares.c
 *
 *  Created on: 18/5/2016
 *      Author: utnso
 */
#include "funcionesAuxiliares.h"
#include <commons/config.h>
#include <commons/string.h>
#include <commons/bitarray.h>
#include <commons/collections/list.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>

#include "initialize.h"

#include "../messageCode/messageCode.h"

int buscarPIDSegunPagInicial(int inicioProg){
	INICIOPROGBUSCADOR= inicioProg;
	t_infoProg* new = (t_infoProg*) (list_find(INFO_PROG, (void*) returnWhenSameInitPage));
	return new->PID;
}

int buscarLongPrograma(int pid){
	PIDBUSCADOR= pid;
	t_infoProg* new = (t_infoProg*) (list_find(INFO_PROG, (void*) returnWhenSamePID));
	return new->LONGITUD;
}

int buscarPagInicial(int pid){
	PIDBUSCADOR=pid;
	t_infoProg* new = (t_infoProg*) (list_find(INFO_PROG, (void*) returnWhenSamePID));
	return new->PAG_INICIAL;
}

void eliminarSegunPID(int pid){
	PIDBUSCADOR= pid;
	list_remove_by_condition(INFO_PROG, (void*)returnWhenSamePID);
	msj_deleteFromINFOPROG(pid);
}

int searchSpace(unsigned programSize){
	int freeSpace =0; 			//PARA REALIZAR COMPACTACION
	int freeSpaceInARow=0;		//PARA ASIGNAR SIN COMPACTAR
	int counter=0;				//CONTADOR DE PAGINAS
	while(counter<CANTIDAD_PAGINAS){
		if(bitarray_test_bit(DISP_PAGINAS, counter)!=0){
			freeSpaceInARow=0;
		}
		else{
			freeSpace++;
			freeSpaceInARow++;
			if(programSize<=freeSpaceInARow)
				return (counter-freeSpaceInARow+1); //DEVUELVE EL NRO DE PAGINA DONDE INICIA EL SEGMENTO LIBRE PARA ASIGNAR EL PROGRAMA
		}
		counter++;
	}
	if(programSize<=freeSpace){
		return -1; //HAY QUE COMPACTAR
	}
	return -2; //NO HAY LUGAR PARA ALBERGARLO
}

void negarEjecucion(){
	send(socketCliente,(void*) NOT_ENOUGH_SPACE,sizeof(NOT_ENOUGH_SPACE),0);
}

void permitirEjecucion(){
	send(socketCliente,(void*) ENOUGH_SPACE,sizeof(ENOUGH_SPACE),0);
}

void infoProg_destroy(t_infoProg *self){
	free(self);
}

int returnWhenSameInitPage (t_infoProg *programa){
	int x = (programa->PAG_INICIAL == INICIOPROGBUSCADOR);
	return x;
}

int returnWhenSamePID(t_infoProg *programa){
	int x = programa->PID == PIDBUSCADOR;
	return x;
}

void msj_Set_Page(int pagina){
	log_trace(logger, "Se ocupo la pagina %d\0", pagina);

}

void msj_Unset_Page(int pagina){

	log_trace(logger, "Se desocupo la pagina %d\0", pagina);

}

void msj_Get_Page(int pagina){

	log_trace(logger, "Se leyo la pagina %d\0", pagina);

}

void msj_Save_Page(int pagina){

	log_trace(logger, "Se guardo la pagina %d\0", pagina);

}

void msj_Save_Program(int pid,int pagInicial,int espacio){

	log_trace(logger, "Se guardo el programa %d desde la pagina %d, hasta la pagina %d\0", pid,pagInicial,pagInicial+espacio);

}

void msj_End_Program(int pid){

	log_trace(logger, "El programa %d ha concluido\0", pid);

}

void msj_A_Compactar(int pid){

	log_trace(logger, "Para albergar el programa %d, se va a proceder a la compactacion\0",pid);

}

void msj_No_Hay_Lugar(int pid){

	log_trace(logger, "El programa %d no se puede albergar\0", pid);

}

void msj_deleteFromINFOPROG(int pid) {

	log_trace(logger, "Se elimino el proceso %d", pid);

}
