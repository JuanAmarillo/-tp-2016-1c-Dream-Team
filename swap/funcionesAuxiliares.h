/*
 * funcionesAuxiliares.h
 *
 *  Created on: 18/5/2016
 *      Author: utnso
 */

#ifndef FUNCIONESAUXILIARES_H_
#define FUNCIONESAUXILIARES_H_

int buscarPIDSegunPagInicial(int inicioProg);
int buscarLongPrograma(int pid);
int buscarPagInicial(int pid);
int searchSpaceToFill(unsigned programSize);
void negarEjecucion();
static void infoProg_destroy(t_infoProg *);
static int returnPIDwhenSameInitPage (t_infoProg *, int );
static int returnWhenSamePID(t_infoProg *);

int PIDBUSCADOR;
int INICIOPROGBUSCADOR;
#endif /* FUNCIONESAUXILIARES_H_ */
