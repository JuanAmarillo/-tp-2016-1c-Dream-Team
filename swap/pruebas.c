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
void setUp();
void mockProceso1();
void mockProceso2();

void pruebaMoverDePosicionPrograma();
void pruebaReservarEspacio();
void pruebaConsistenciaDatos();

int main(){
	//pruebaMoverDePosicionPrograma();
	//pruebaReservarEspacio();
	pruebaConsistenciaDatos();



	return 0;
};

void setUp() {
	initialConf();
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

	setUp();
	int pagInicial = buscarPagInicial(2);

	moveProgram(4,2);

	int buscarPID =  buscarPIDSegunPagInicial(2);

	int primerPagFinal = buscarPagInicial(2);

	printf("El programa %d, se movio de %d a %d", buscarPID , pagInicial, primerPagFinal);
}



void pruebaReservarEspacio(){
	setUp();
	unsigned parametros[2];
	parametros[0] = 3;
	parametros[1] = 2;
	received.parametros = parametros;

	reservarEspacio();
}



void pruebaConsistenciaDatos(){
	setUp();
	char pagina[10];
	strcpy(pagina,"HolaGato\0");
	strcpy(bufferPagina,pagina);
	savePage(3);
	strcpy(bufferPagina,"\0");
	getPage(3);
	puts("gets");
	printf("El contenido de la pagina es: %s .\n", bufferPagina);
	log_trace(logger,bufferPagina);
	accionesDeFinalizacion();
	//pruebaReservarEspacio();
}
