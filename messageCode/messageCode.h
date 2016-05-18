/*
 * messageCode.h
 *
 *  Created on: 18/5/2016
 *      Author: utnso
 */

#ifndef MESSAGECODE_H_
#define MESSAGECODE_H_



// DESDE LA UMC, LO QUE RECIBE EL SWAP
#define RESERVE_SPACE "900" 	// Parametros recibidos despues de la cabecera: pid(unsigned), longitud del programa(unsigned)
#define SAVE_PROGRAM "905"		// Parametros recibidos despues de la cabecera: pid(unsigned), paginas(char*)
#define SAVE_PAGE "910"			// Parametros recibidos despues de la cabecera: pid(unsigned), numeroDePagDentroDelPrograma(unsigned)
#define END_PROGRAM "915"		// Parametros recibidos despues de la cabecera: pid(unsigned)
#define RETURN_PAGE "920"		// Parametros recibidos despues de la cabecera: pid(unsigned), numeroDePagDentroDelPrograma(unsigned)
#define NOT_ENOUGH_SPACE "930" 	// Parametros recibidos despues de la cabecera: pid(unsigned)

#endif /* MESSAGECODE_H_ */
