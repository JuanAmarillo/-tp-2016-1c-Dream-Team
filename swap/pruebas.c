/*
 * pruebas.c
 *
 *  Created on: 1/7/2016
 *      Author: utnso
 */
#include "swap.h"
#include "initialize.h"
#include "funcionesAuxiliares.h"
#include <commons/string.h>
#include <stdio.h>
#include "../messageCode/messageCode.h"

void pruebaSetUp();
void mockProceso1();
void mockProceso2();


void pruebaMoverDePosicionPrograma();
void pruebaReservarEspacio();
void pruebaConsistenciaDatos();
void pruebaGenerarReceive();
void pruebaProcesoGuardarPrograma();
void pruebaProcesoConectar();
void pruebaBuscarLongitud();
void pruebaRecibirMensaje();
void pruebaFUNCIONALIDADCOMPLETA();
/*
int main(){
	//pruebaMoverDePosicionPrograma();
	//pruebaReservarEspacio();
	//pruebaConsistenciaDatos();
	//pruebaProcesoGuardarPrograma();
	//pruebaProcesoConectar();
	//pruebaBuscarLongitud();
	//pruebaRecibirMensaje();
	pruebaFUNCIONALIDADCOMPLETA();
	accionesDeFinalizacion();
	return 0;
};
*/
void pruebaSetUp() {
	mockProceso1();
	mockProceso2();
}

void mockProceso1(){
	asignarEspacio(1,0,2);
}

void mockProceso2(){
	asignarEspacio(2,3,2);
}


void pruebaMoverDePosicionPrograma(){

	pruebaSetUp();
	int pagInicial = buscarPagInicial(2);

	moveProgram(4,2);

	int buscarPID =  buscarPIDSegunPagInicial(2);

	int primerPagFinal = buscarPagInicial(2);

	printf("El programa %d, se movio de %d a %d", buscarPID , pagInicial, primerPagFinal);
}

void pruebaReservarEspacio(){
	pruebaSetUp();
	unsigned parametros[2];
	parametros[0] = 58;
	parametros[1] = 2;
	received.parametros = parametros;

	reservarEspacio();
}

void pruebaConsistenciaDatos(){
	pruebaSetUp();
	char pagina[10];
	strcpy(pagina,"HolaGato\0");
	strcpy(bufferPagina,pagina);
	savePage(3);
	strcpy(bufferPagina,"\0");
	puts("Lee pagina vacia");
	getPage(2);
	printf("El contenido de la pagina es: %s .\n", (char*) bufferPagina);
	puts("Lee pagina ocupada");
	getPage(3);
	printf("El contenido de la pagina es: %s .\n", (char*) bufferPagina);
	log_trace(logger,bufferPagina);

}

void pruebaGenerarReceive(){
	t_mensajeHead header;
	header.cantidad_parametros = 2;
	header.codigo = RESERVE_SPACE;
	header.tam_extra = 0;
	unsigned parametros[2];
	parametros[0] = 30;
	parametros[1] = 2;
	t_mensaje mensaje;
	mensaje.head = header;
	mensaje.parametros = parametros;
	mensaje.mensaje_extra = NULL;
	received = mensaje;
}

void pruebaProcesoGuardarPrograma(){
	pruebaSetUp();
	pruebaGenerarReceive();
	//puts("Hola");
	funcionamientoSWAP();
}

void pruebaProcesoConectar(){
	pruebaSetUp();
	socketConf();
}

void pruebaBuscarLongitud(){
	initialConf();
	pruebaSetUp();
	int a = buscarPagInicial(2);
	printf("La pagina inicial del proceso 2 es %d\n",a);
	int b = buscarPIDSegunPagInicial(3);
	printf("El PID es %d\n",b);
	int c = buscarLongPrograma(2);
	printf("La longitud es %d\n",c);
}

void pruebaRecibirMensaje(){
	initialConf();
	socketConf();
	pruebaSetUp();
	puts("Antes del funcionamiento");
	int a = funcionamientoSWAP();
	printf("%d",a);
	puts("Despuyes del funcionamiento");

}

void pruebaFUNCIONALIDADCOMPLETA(){
	initialConf();
	socketConf();
	pruebaSetUp();
	int i;
	for(i=0; i<5; i++)
		funcionamientoSWAP();
	puts("finfuncionamiento");
	getPage(5);
	char* palabra = malloc(TAMANIO_PAGINA+1);
	strcpy(palabra,bufferPagina);
	palabra[TAMANIO_PAGINA+1] = '\0';
	printf("El contenido de la pagina es %s\n\n", palabra);
	int a = buscarPagInicial(3);
	if(a == -1)
		printf("El programa se borro");
	else
		printf("El programa no se borro y su pag inicial es %d",a);
}

