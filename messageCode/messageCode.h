/*
 * messageCode.h
 *
 *  Created on: 18/5/2016
 *      Author: utnso
 */

#ifndef MESSAGECODE_H_
#define MESSAGECODE_H_



// DESDE LA UMC, LO QUE RECIBE EL SWAP
#define RESERVE_SPACE 900 	// parametros: pid(unsigned), longitud del programa(unsigned)
#define SAVE_PROGRAM 905	// parametros: pid(unsigned), paginas(char*)
#define SAVE_PAGE 910		// parametros: pid(unsigned), numeroDePagDentroDelPrograma(unsigned)
#define END_PROGRAM 915		// parametros: pid(unsigned)
#define RETURN_PAGE 920		// parametros: pid(unsigned), numeroDePagDentroDelPrograma(unsigned)


#endif /* MESSAGECODE_H_ */
