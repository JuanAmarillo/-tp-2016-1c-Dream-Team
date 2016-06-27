/*
 * messageCode.h
 *
 *  Created on: 18/5/2016
 *      Author: utnso
 */

#ifndef MESSAGECODE_H_
#define MESSAGECODE_H_

//CONSOLA -> NUCLEO
#define NUEVO_PROGRAMA 100
//NUCLEO -> CONSOLA
//#define SALIDA_PROGRAMA 101
#define IMPRIMIR_PROGRAMA 101
#define IMPRIMIR_TEXTO_PROGAMA 102


// CPU -> UMC
#define GET_TAM_PAGINA 700 
#define GET_DATA 701
#define RESERVE_MEMORY 702
#define RECORD_DATA 703

// UMC -> CPU
#define RETURN_DATA 704
#define RETURN_TAM_PAGINA 705
#define RETURN_POS 706
#define RETURN_RECORD_DATA 707

// NUCLEO <- CPU
#define STRUCT_PCB 708
#define STRUCT_PCB_FIN 709
#define STRUCT_PCB_FIN_ERROR 710
#define OBTENER_COMPARTIDA 711
#define ASIGNAR_COMPARTIDA 712
#define IMPRIMIR 713
#define IMPRIMIR_TEXTO 714
#define ENTRADA_SALIDA 715
#define WAIT 716
#define SIGNAL 717

// NUCLEO -> CPU
#define RETURN_OBTENER_COMPARTIDA 303        //PCB para ejecutar
#define QUANTUM 304         //Parámetros: quantum, quantumSleep(retardo de cada instruccion, usar sleep(quantumSleep);)

//NUCLEO -> UMC
#define INIT_PROG 300
#define FIN_PROG 305

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
