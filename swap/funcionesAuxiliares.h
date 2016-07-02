/*
 * funcionesAuxiliares.h
 *
 *  Created on: 18/5/2016
 *      Author: utnso
 */

#ifndef FUNCIONESAUXILIARES_H_
#define FUNCIONESAUXILIARES_H_
#include "swap.h"

int buscarPIDSegunPagInicial(int);
int buscarLongPrograma(int);
int buscarPagInicial(int);
void eliminarSegunPID(int pid);
/*
 *	La funcion busca los espacios disponibles en el archivo SWAP para asignar un proyecto.
 *	Si la funcion devuelve 0, o un numero positivo, este es el numero de pagina donde inicia el segmento de paginas vacias para asignar el proyecto.
 *	Si la funcion devuelve -1, se debe realizar una compactacion para asignar el proyecto.
 *	Si la funcion devuelve -2, no hay memoria disponible en el SWAP y se debe avisar a la UMC para rechazar el programa.
 **/
int searchSpace(unsigned);
void negarEjecucion();
void permitirEjecucion();
void infoProg_destroy(t_infoProg*);
int returnWhenSameInitPage (t_infoProg*);
int returnWhenSamePID(t_infoProg*);
void msj_Set_Page(int);
void msj_Unset_Page(int);
void msj_Get_Page(int);
void msj_Save_Page(int);
void msj_Save_Program(int,int,int);
void msj_End_Program(int);
void msj_A_Compactar(int);
void msj_No_Hay_Lugar(int);
void msj_deleteFromINFOPROG(int );
void msj_addToInfoProg(int PID);
void msj_Reservar_Espacio(int,int );
int PIDBUSCADOR;
int INICIOPROGBUSCADOR;
#endif /* FUNCIONESAUXILIARES_H_ */
