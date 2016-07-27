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
	if(new) return new->PID;
	else return -1;
}

int buscarLongPrograma(int pid){
	PIDBUSCADOR= pid;
	t_infoProg* new = (t_infoProg*) (list_find(INFO_PROG, (void*) returnWhenSamePID));
	if(new) return new->LONGITUD;
	else return -1;
}

int buscarPagInicial(int pid){
	PIDBUSCADOR=pid;
	t_infoProg* new = (t_infoProg*) (list_find(INFO_PROG, (void*) returnWhenSamePID));
	if(new) return new->PAG_INICIAL;
	else return -1;
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
	t_mensaje mensaje;
	mensaje.head.codigo =NOT_ENOUGH_SPACE;
	mensaje.head.tam_extra = 0;
	mensaje.head.cantidad_parametros = 0;
	mensaje.parametros = NULL;
	mensaje.mensaje_extra = NULL;
	enviarMensaje(socketCliente,mensaje);
}
void permitirEjecucion(){
	t_mensaje mensaje;
	mensaje.head.codigo =ENOUGH_SPACE;
	mensaje.head.tam_extra = 0;
	mensaje.head.cantidad_parametros = 0;
	mensaje.parametros = NULL;
	mensaje.mensaje_extra = NULL;
	enviarMensaje(socketCliente,mensaje);
	log_trace(logger, "Se le permite el almacenamiento");
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
	log_trace(logger, "Se ocupo la pagina %d", pagina);
}

void msj_Unset_Page(int pagina){
	log_trace(logger, "Se desocupo la pagina %d", pagina);
}

void msj_Get_Page(int pagina){
	log_trace(logger, "Se leyo la pagina %d", pagina);
}

void msj_Save_Page(int pagina){
	log_trace(logger, "Se guardo la pagina %d", pagina);
}

void msj_Save_Program(int pid,int pagInicial,int espacio){
	log_trace(logger, "Se guardo el programa %d desde la pagina %d, hasta la pagina %d", pid,pagInicial,pagInicial+espacio-1);
}

void msj_End_Program(int pid){
	log_trace(logger, "El programa %d ha concluido", pid);
}

void msj_A_Compactar(int pid){
	log_trace(logger, "Para albergar el programa %d, se va a proceder a la compactacion",pid);
}

void msj_No_Hay_Lugar(int pid){
	log_trace(logger, "El programa %d no se puede albergar", pid);
}

void msj_deleteFromINFOPROG(int pid) {
	log_trace(logger, "Se elimino el proceso %d del INFOPROG", pid);
}

void msj_addToInfoProg(int PID){
	log_trace(logger, "Se agrego el proceso %d al INFOPROG", PID);
}

void msj_Reservar_Espacio(int pid,int longitud){
	log_trace(logger, "Se solicita la reserva de espacio del proceso %d de longitud %d", pid, longitud);
}
