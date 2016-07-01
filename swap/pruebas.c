/*
 * pruebas.c
 *
 *  Created on: 1/7/2016
 *      Author: utnso
 */
#include "swap.h"
#include "initialize.h"
#include "funcionesAuxiliares.h"

#include <stdio.h>

void pruebaMoverDePosicionPrograma();
void mockProceso1();
void mockProceso2();

int main(){
	pruebaMoverDePosicionPrograma();
	return 0;
};


void pruebaMoverDePosicionPrograma(){

	initialConf();

	mockProceso1();

	mockProceso2();
	puts("Hola");
	int pagInicial = buscarPagInicial(2);

	moveProgram(4,2);

	int buscarPID =  buscarPIDSegunPagInicial(2);

	int primerPagFinal = buscarPagInicial(2);

	printf("El programa %d, se movio de %d a %d", buscarPID , pagInicial, primerPagFinal);
}

void mockProceso1(){
	asignarEspacio(1,0,2);
}

void mockProceso2(){
	asignarEspacio(2,4,2);
}
