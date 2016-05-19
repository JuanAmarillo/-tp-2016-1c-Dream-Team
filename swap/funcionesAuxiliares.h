/*
 * funcionesAuxiliares.h
 *
 *  Created on: 18/5/2016
 *      Author: utnso
 */

#ifndef FUNCIONESAUXILIARES_H_
#define FUNCIONESAUXILIARES_H_

t_infoProg buscarPIDSegunPagInicial(int inicioProg);
int buscarLongPrograma(int pid);
int buscarPagInicial(int pid);
int searchSpaceToFill(unsigned programSize);
void negarEjecucion();
static void infoProg_destroy(t_infoProg *);
static bool returnPIDwhenSameInitPage (int , t_infoProg *);
#endif /* FUNCIONESAUXILIARES_H_ */
