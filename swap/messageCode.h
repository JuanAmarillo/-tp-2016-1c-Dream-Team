/*
 * messageCode.h
 *
 *  Created on: 18/5/2016
 *      Author: utnso
 */

#ifndef MESSAGECODE_H_
#define MESSAGECODE_H_



// DESDE LA UMC, LO QUE RECIBE EL SWAP
#define RESERVE_SPACE 900 		// Parametros recibidos despues de la cabecera: pid(unsigned), longitud del programa(unsigned)
#define SAVE_PROGRAM 905		// Parametros recibidos despues de la cabecera: pid(unsigned), paginas(char[TAMANIO_PAGINA])
#define SAVE_PAGE 910			// Parametros recibidos despues de la cabecera: pid(unsigned), numeroDePagDentroDelPrograma(unsigned), pagina(char[TAMANIO_PAGINA])
#define END_PROGRAM 915			// Parametros recibidos despues de la cabecera: pid(unsigned)
#define BRING_PAGE_TO_UMC 920	// Parametros recibidos despues de la cabecera: pid(unsigned), numeroDePagDentroDelPrograma(unsigned)
#define NOT_ENOUGH_SPACE 925 	// Parametros recibidos despues de la cabecera: pid(unsigned)

// DESDE EL SWAP, LO QUE RECIBE LA UMC
#define SWAP_SENDS_PAGE 930		// Parametros recibidos despues de la cabecera: pagina(char[TAMANIO_PAGINA])
#endif /* MESSAGECODE_H_ */
