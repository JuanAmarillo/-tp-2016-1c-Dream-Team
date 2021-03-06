/*
 * swap.c
 *
 *  Created on: 26/4/2016
 *      Author: utnso
 */
#include "../messageCode/messageCode.h"
#include "initialize.h"
#include "funcionesAuxiliares.h"


#include <commons/config.h>
#include <commons/string.h>
#include <netdb.h>
#include <unistd.h>
#include "swap.h"


int main(){
	initialConf();
	socketConf();
	while (funcionamientoSWAP()!=0);
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

void limpiarMensaje(){
	received.head.cantidad_parametros = 0;
	received.head.codigo = 0;
	received.parametros = 0;
	received.mensaje_extra = NULL;
	received.parametros = NULL;
}

int funcionamientoSWAP() {
		int a = recibirMensaje(socketCliente, &received);
		if(a!=0){
			switch (received.head.codigo) {
				case RESERVE_SPACE:
					reservarEspacio();
					break;
				case SAVE_PROGRAM:
					saveProgram();
					break;
				case SAVE_PAGE:
					saveNewPage();
					break;
				case FIN_PROG:
					endProgram();
					break;
				case BRING_PAGE_TO_UMC:
					returnPage();
					break;
			}
		}
		freeMensaje(&received);
		limpiarMensaje();
		return a;
	}

void enviarMensajeFinSaveProgram(int pid){
	t_mensaje mensaje;
	mensaje.head.codigo = FIN_SAVE_PROGRAM;
	mensaje.head.cantidad_parametros = 0;
	mensaje.head.tam_extra = 0;
	mensaje.parametros = NULL;
	mensaje.mensaje_extra = NULL;
	enviarMensaje(socketCliente,mensaje);
}

void setNewPage(unsigned nroPag){
	bitarray_set_bit(DISP_PAGINAS, nroPag);
	msj_Set_Page(nroPag);
}

void unSetPage(unsigned nroPag){
	bitarray_clean_bit(DISP_PAGINAS,nroPag);
	msj_Unset_Page(nroPag);
}

void getPage(unsigned nroPag){
	fseek(SWAPFILE,nroPag*TAMANIO_PAGINA,SEEK_SET);
	fread(bufferPagina,1,TAMANIO_PAGINA,SWAPFILE);
	usleep(RETARDO_ACCESO);
}

void savePage(unsigned nroPag){
	fseek(SWAPFILE,nroPag*TAMANIO_PAGINA,SEEK_SET);
	fwrite(bufferPagina,1,TAMANIO_PAGINA,SWAPFILE);
	usleep(RETARDO_ACCESO);
}

void saveProgram(){
	int espacio, pagInicial, cantidadGuardada=0;

	espacio = buscarLongPrograma(received.parametros[0]);
	pagInicial = buscarPagInicial(received.parametros[0]);
	log_trace(logger,"El codigo de la pagina es:\n");
	log_trace(logger,received.mensaje_extra);
	int tam_a_copiar;
	while((received.head.tam_extra - cantidadGuardada*TAMANIO_PAGINA) > 0){
		if((received.head.tam_extra - cantidadGuardada*TAMANIO_PAGINA) >= TAMANIO_PAGINA){
			tam_a_copiar = TAMANIO_PAGINA;
		} else {
			tam_a_copiar = received.head.tam_extra - cantidadGuardada*TAMANIO_PAGINA;
		}
		memcpy(bufferPagina, received.mensaje_extra + cantidadGuardada*TAMANIO_PAGINA, tam_a_copiar);
		if(tam_a_copiar != TAMANIO_PAGINA) memset(bufferPagina + tam_a_copiar, '\0', tam_a_copiar-TAMANIO_PAGINA);
		savePage(pagInicial+cantidadGuardada);
		cantidadGuardada++;
	}
	enviarMensajeFinSaveProgram(received.parametros[0]);
	msj_Save_Program(received.parametros[0],pagInicial,espacio);
}

void returnPage(){
	t_mensaje aEnviar;
	aEnviar.head.codigo = SWAP_SENDS_PAGE;
	aEnviar.head.cantidad_parametros = 0;
	aEnviar.head.tam_extra = TAMANIO_PAGINA;
	aEnviar.mensaje_extra = malloc(TAMANIO_PAGINA);
	aEnviar.parametros = NULL;
	getPage(buscarPagInicial(received.parametros[0])+received.parametros[1]);
	msj_Get_Page(buscarPagInicial(received.parametros[0])+received.parametros[1], received.parametros[0]);
	memcpy(aEnviar.mensaje_extra, bufferPagina, TAMANIO_PAGINA);
	enviarMensaje(socketCliente, aEnviar);
	free(aEnviar.mensaje_extra);
}

void endProgram(){
	int longitud, inicial,contador= 0;
	longitud = buscarLongPrograma(received.parametros[0]);
	inicial = buscarPagInicial(received.parametros[0]);
	while(contador<longitud){
		unSetPage(inicial+contador);
		contador++;
	}
	eliminarSegunPID(received.parametros[0]);
	msj_End_Program(received.parametros[0]);
}

void saveNewPage(){
	int nroPagDentroProg = received.parametros[1];
	int pagInicial = buscarPagInicial(received.parametros[0]);
	memcpy(bufferPagina, received.mensaje_extra, TAMANIO_PAGINA);
	savePage(pagInicial+nroPagDentroProg);
	msj_Save_Page(pagInicial+nroPagDentroProg, received.parametros[0]);
}

void replacePages(int longitudPrograma, int inicioProg,int inicioEspacioBlanco) {
	int contador=0;
	while (contador < longitudPrograma) {
		getPage(inicioProg + contador);
		unSetPage(inicioProg + contador);
		savePage(inicioEspacioBlanco + contador);
		setNewPage(inicioEspacioBlanco + contador);
		contador++;
	}

}

void agregarAlINFOPROG(t_infoProg* new) {
	list_add(INFO_PROG, (void*) new);
	msj_addToInfoProg(new->PID);
}

void new_Or_Replace_t_infoProg(int pid, int inicioProg, int longitudPrograma,int eliminar) {
	t_infoProg* new = malloc(sizeof(t_infoProg));
	new->PID = pid;
	new->LONGITUD = longitudPrograma;
	new->PAG_INICIAL = inicioProg;
	if(eliminar)
		eliminarSegunPID(pid);
	agregarAlINFOPROG(new);

}

void asignarEspacio(unsigned pid, int lugar, unsigned tamanio){
	new_Or_Replace_t_infoProg(pid,lugar,tamanio,0);
	int paginasReservadas=0;
	while(paginasReservadas< tamanio){
		setNewPage(lugar);
		lugar++;
		paginasReservadas++;
	}
}

void mostrarDisponibilidad(){
	int i = 0;
	for(i=0;i<CANTIDAD_PAGINAS;i++){
		log_trace(logger1,"Pag: %i, Estado: %i",i,bitarray_test_bit(DISP_PAGINAS,i));
	}
}

void reservarEspacio(){
	msj_Reservar_Espacio(received.parametros[0], received.parametros[1]);
	int lugar = searchSpace(received.parametros[1]);
	if(lugar == -2){
			negarEjecucion();
			msj_No_Hay_Lugar(received.parametros[0]);
			return;
		}
	permitirEjecucion();
	if(lugar == -1){
		msj_A_Compactar(received.parametros[0]);
		deleteEmptySpaces();
		lugar = searchSpace(received.parametros[1]);
	}
	asignarEspacio(received.parametros[0],lugar,received.parametros[1]);
	mostrarDisponibilidad();
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
	usleep(RETARDO_COMPACTACION);
}
