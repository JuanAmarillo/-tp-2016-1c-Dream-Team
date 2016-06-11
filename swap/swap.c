/*
 * swap.c
 *
 *  Created on: 26/4/2016
 *      Author: utnso
 */
#include <commons/config.h>
#include <commons/string.h>
#include <commons/bitarray.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include "swap.h"
#include "../messageCode/messageCode.h"
#include "initialize.h"
#include "funcionesAuxiliares.h"
#include "CUnit/Basic.h"

int main(){
	readConfigFile();
	crearArchivoSWAP();
	crearEstructurasDeManejo();
	setSocket();
	bindSocket();
	acceptSocket();
	int cabecera = recibirCabecera();
	while(cabecera){
		switch(cabecera){
			case RESERVE_SPACE: reservarEspacio();
				break;
			case SAVE_PROGRAM: saveProgram();
				break;
			case SAVE_PAGE: saveNewPage();
				break;
			case END_PROGRAM: endProgram();
				break;
			case BRING_PAGE_TO_UMC: returnPage();
				break;
		}
		cabecera = recibirCabecera();
	}
	accionesDeFinalizacion();
	return 0;
}

void setNewPage(unsigned nroPag){
	bitarray_set_bit(DISP_PAGINAS, nroPag);
}

void unSetPage(unsigned nroPag){
	bitarray_clean_bit(DISP_PAGINAS,nroPag);
}

void getPage(unsigned nroPag){
	fseek(SWAPFILE,nroPag*TAMANIO_PAGINA,SEEK_SET);
	sleep(RETARDO_ACCESO);
	fread(paginaMultiProposito,TAMANIO_PAGINA,1,SWAPFILE);
}

void savePage(unsigned nroPag){
	fseek(SWAPFILE,nroPag*TAMANIO_PAGINA,SEEK_SET);
	sleep(RETARDO_ACCESO);
	fwrite(paginaMultiProposito,TAMANIO_PAGINA,1,SWAPFILE);
}

int recibirCabecera(){
	int codigo = 0;
	recv(socketCliente, &codigo, 4, 0);
	return codigo;
}

void overWritePage(int nroPag){
	fseek(SWAPFILE,nroPag*TAMANIO_PAGINA,SEEK_SET);
	char* barraCero = "\0";
	fwrite(barraCero,1,TAMANIO_PAGINA,SWAPFILE);
	unSetPage(nroPag);
}

void saveProgram(){
	int espacio, pagInicial, cantidadGuardada=1;
	unsigned pid;
	recv(socketCliente,&pid,4,0);
	espacio = buscarLongPrograma(pid);
	pagInicial = buscarPagInicial(pid);
	while(cantidadGuardada<=espacio){
		recv(socketCliente,paginaMultiProposito,TAMANIO_PAGINA,0);
		savePage(pagInicial+cantidadGuardada);
		cantidadGuardada++;
	}
}

void returnPage(){
	unsigned pid, nroPagDentroProg;
	recv(socketCliente,&pid,4,0);
	recv(socketCliente,&nroPagDentroProg,4,0);
	getPage(buscarPagInicial(pid)+nroPagDentroProg);
	send(socketCliente,(void *)SWAP_SENDS_PAGE,4,0);
	send(socketCliente,paginaMultiProposito,TAMANIO_PAGINA,0);
}

void endProgram(){
	int pid, longitud, inicial, contador=1;
	recv(socketCliente,&pid,4,0);
	longitud = buscarLongPrograma(pid);
	inicial = buscarPagInicial(pid);
	eliminarSegunPID(pid);
	while(contador<=longitud){
		overWritePage(inicial+contador);
	}
}

void saveNewPage(){
	unsigned pid;
	unsigned nroPagDentroProg;
	recv(socketCliente,&pid,4,0);
	recv(socketCliente,&nroPagDentroProg,4,0);
	recv(socketCliente,paginaMultiProposito,TAMANIO_PAGINA,0);
	int pagInicial = buscarPagInicial(pid);
	savePage(pagInicial+nroPagDentroProg);
}

void replacePages(int longitudPrograma, int inicioProg,int inicioEspacioBlanco) {
	int contador=0;
	while (contador <= longitudPrograma) {
		getPage(inicioProg + contador);
		savePage(inicioEspacioBlanco + contador);
		unSetPage(inicioProg + contador);
		setNewPage(inicioEspacioBlanco + contador);
		contador++;
	}
}

void new_Or_Replace_t_infoProg(int pid, int longitudPrograma, int inicioProg,int eliminar) {
	t_infoProg* new = malloc(sizeof(t_infoProg));
	new->PID = pid;
	new->LONGITUD = longitudPrograma;
	new->PAG_INICIAL = inicioProg;
	if(eliminar)
		eliminarSegunPID(pid);
	list_add(INFO_PROG, (void*) new);
}

void asignarEspacio(unsigned pid, int lugar, unsigned tamanio){
	new_Or_Replace_t_infoProg(pid,lugar,tamanio,0);
	int paginasReservadas;
	while(paginasReservadas<= tamanio){
		setNewPage(lugar);
		lugar++;
		paginasReservadas++;
	}
}

void reservarEspacio(){
	unsigned pid, espacio;
	recv(socketCliente, &pid, 4, 0);
	recv(socketCliente, &espacio, 4, 0);
	int lugar = searchSpace(espacio);
	if(lugar == -1){
		deleteEmptySpaces();
		lugar = searchSpace(espacio);
	}
	if(lugar == -2){
		negarEjecucion(pid);
		return;
	}
	else
		permitirEjecucion(pid);
	asignarEspacio(pid,lugar,espacio);
}

void moveProgram(int inicioProg, int inicioEspacioBlanco){

	int pid = buscarPIDSegunPagInicial(inicioProg);
	int longitudPrograma= buscarLongPrograma(pid);
	new_Or_Replace_t_infoProg(pid, longitudPrograma, inicioProg, 1);
	replacePages(longitudPrograma, inicioProg, inicioEspacioBlanco);
}

void deleteEmptySpaces(){
	int contador=0, inicioEspacioBlanco =0, estadoPagina;
	int espacioBlancoDetras=0;
	while(contador<CANTIDAD_PAGINAS){
		estadoPagina=bitarray_test_bit(DISP_PAGINAS,contador);
		if(estadoPagina==0){
			if(!espacioBlancoDetras){
				inicioEspacioBlanco=contador;
			}
			espacioBlancoDetras=1;
			contador++;
		}
		else if(espacioBlancoDetras){
			moveProgram(contador,inicioEspacioBlanco);
			espacioBlancoDetras=0;
		}
		else
			contador++;
	}
	sleep(RETARDO_COMPACTACION);
}
