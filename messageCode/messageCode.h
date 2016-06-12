/*
 * messageCode.h
 *
 *  Created on: 18/5/2016
 *      Author: utnso
 */

#ifndef MESSAGECODE_H_
#define MESSAGECODE_H_

// CPU -> UMC
#define GET_DATA 200 			// Parametros: pagina, offset, size
#define GET_TAM_PAGINA 205 		// Parametros: NULL
#define RESERVE_MEMORY 210 		// Parametros: TamañoMemoriaReservar ;
#define RECORD_DATA 215 		// Parametros: numero_pagina, desplazamiento, tamañano, data

//CONSOLA -> NUCLEO
#define NUEVO_PROGRAMA 100
//NUCLEO -> CONSOLA
//#define SALIDA_PROGRAMA 101
#define IMPRIMIR_PROGRAMA 101
#define IMPRIMIR_TEXTO_PROGAMA 102

// NUCLEO <- CPU
#define STRUCT_PCB 300 			// Utilizar la funcion: mensaje_to_pcb() y pcb_to_mensaje() segun corresponda
#define FIN_QUANTUM 301
#define FIN_PROGRAMA 302
// NUCLEO -> CPU
#define EJECUTAR 303        //PCB para ejecutar
#define QUANTUM 304         //Parámetros: quantum, quantumSleep(retardo de cada instruccion, usar sleep(quantumSleep);)

//NUCLEO -> UMC
#define INIT_PROG 300
#define FIN_PROG 305

// UMC -> CPU
#define RETURN_DATA 201 		// Parametros: NULL; mensaje_extra: Bytes en memoria solicitados
#define RETURN_TAM_PAGINA 206 	// Parametros: tamaño_pagina
#define RETURN_POS 211 			// Parametros: numero_pagina, desplazamiento, tamaño
#define RECORD_OK 216			// Parametros: null

// DESDE LA UMC, LO QUE RECIBE EL SWAP
#define RESERVE_SPACE 900 		// Parametros recibidos despues de la cabecera: pid(unsigned), longitud del programa(unsigned)
#define SAVE_PROGRAM 905		// Parametros recibidos despues de la cabecera: pid(unsigned), paginas(char[TAMANIO_PAGINA])
#define SAVE_PAGE 910			// Parametros recibidos despues de la cabecera: pid(unsigned), numeroDePagDentroDelPrograma(unsigned), pagina(char[TAMANIO_PAGINA])
#define END_PROGRAM 915			// Parametros recibidos despues de la cabecera: pid(unsigned)
#define BRING_PAGE_TO_UMC 920	// Parametros recibidos despues de la cabecera: pid(unsigned), numeroDePagDentroDelPrograma(unsigned)
#define NOT_ENOUGH_SPACE 925 	// Parametros recibidos despues de la cabecera: pid(unsigned)

// DESDE EL SWAP, LO QUE RECIBE LA UMC
#define SWAP_SENDS_PAGE 930		// Parametros recibidos despues de la cabecera: pagina(char[TAMANIO_PAGINA])
#define ENOUGH_SPACE 955		  // Parametros recibidos despues de la cabecera: pid(unsigned)
#endif /* MESSAGECODE_H_ */
