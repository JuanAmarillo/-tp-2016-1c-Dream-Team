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
#include "../cpu/protocolo_mensaje.h"

int main(){
	initialConf();
	socketConf();
	int cabecera = recibirCabecera();
	while(1){
		recibirMensaje(socketCliente,&received);
		switch(received.head.codigo){
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

void socketConf() {
	setSocket();
	bindSocket();
	acceptSocket();
}

void initialConf() {
	readConfigFile();
	crearArchivoSWAP();
	crearEstructurasDeManejo();
}

void setNewPage(unsigned nroPag){
	bitarray_set_bit(DISP_PAGINAS, nroPag);
}

void unSetPage(unsigned nroPag){
	bitarray_clean_bit(DISP_PAGINAS,nroPag);
}

void *getPage(unsigned nroPag){
	void * pagina = malloc(TAMANIO_PAGINA);
	fseek(SWAPFILE,nroPag*TAMANIO_PAGINA,SEEK_SET);
	sleep(RETARDO_ACCESO);
	fread(pagina,TAMANIO_PAGINA,1,SWAPFILE);
	return pagina;
}

void savePage(unsigned nroPag,char* pagina){
	fseek(SWAPFILE,nroPag*TAMANIO_PAGINA,SEEK_SET);
	sleep(RETARDO_ACCESO);
	fwrite(pagina,TAMANIO_PAGINA,1,SWAPFILE);
}

void saveProgram(){
	int espacio, pagInicial, cantidadGuardada=0;
	espacio = buscarLongPrograma(received.parametros[0]);
	pagInicial = buscarPagInicial(received.parametros[0]);
	while(cantidadGuardada<espacio){
		savePage(pagInicial+cantidadGuardada,received.mensaje_extra[cantidadGuardada*TAMANIO_PAGINA]);
		cantidadGuardada++;
	}
}

void returnPage(){
	t_mensaje aEnviar;
	aEnviar.head.codigo = SWAP_SENDS_PAGE;
	aEnviar.head.cantidad_parametros = 0;
	aEnviar.head.tam_extra = TAMANIO_PAGINA;
	aEnviar.mensaje_extra = getPage(buscarPagInicial(received.parametros[0])+received.parametros[1]);
}

void endProgram(){
	int longitud, inicial, contador=1;
	longitud = buscarLongPrograma(received.parametros[0]);
	inicial = buscarPagInicial(received.parametros[0]);
	eliminarSegunPID(received.parametros[0]);
}

void saveNewPage(){
	int nroPagDentroProg = received.parametros[1];
	int pagInicial = buscarPagInicial(received.parametros[0]);
	savePage(pagInicial+nroPagDentroProg,received.mensaje_extra);
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
	int lugar = searchSpace(received.parametros[1]);
	if(lugar == -1){
		deleteEmptySpaces();
		lugar = searchSpace(received.parametros[1]);
	}
	if(lugar == -2){
		negarEjecucion(received.parametros[0]);
		return;
	}
	else
		permitirEjecucion(received.parametros[0]);
	asignarEspacio(received.parametros[0],lugar,received.parametros[1]);
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
