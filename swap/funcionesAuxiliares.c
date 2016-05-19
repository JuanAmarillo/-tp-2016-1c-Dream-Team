/*
 * funcionesAuxiliares.c
 *
 *  Created on: 18/5/2016
 *      Author: utnso
 */
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
#include "swap.h"
#include "messageCode.h"
#include "initialize.h"
#include "funcionesAuxiliares.h"

t_infoProg buscarPIDSegunPagInicial(int inicioProg){
	return (list_find(INFO_PROG,(void*) returnPIDwhenSameInitPage(inicioProg)));
}

int buscarLongPrograma(int pid){
	int i=0;
		while(i<=CANTIDAD_PAGINAS){
			if(pid == INFO_PROG[i].PID)
				return INFO_PROG[i].LONGITUD;
			i++;
		}
		return -1;
}

int buscarPagInicial(int pid){
	int i=0;
	while(i<=CANTIDAD_PAGINAS){
		if(pid == INFO_PROG[i].PID)
			return INFO_PROG[i].PAG_INICIAL;
		i++;
	}
	return -1;
}

int searchSpaceToFill(unsigned programSize){
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

static void infoProg_destroy(t_infoProg *self){
	free(self);
}

static bool returnPIDwhenSameInitPage (t_infoProg *programa, int pagInicial){
	bool x = (programa->PAG_INICIAL == pagInicial);
	return x;
}
