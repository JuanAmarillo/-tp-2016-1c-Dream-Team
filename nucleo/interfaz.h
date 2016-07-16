#ifndef INTERFAZ_H_
#define INTERFAZ_H_

#include <stdio.h>
#include <stdlib.h>

#include "funciones_nucleo.h"
#include "pcb.h"
#include "planificador.h"

void interfaz(void);
void imprimirListos(void);
void imprimirBloqueados(void);
void imprimirMaster(void);
void imprimirListaProcesos(const t_list*);
void imprimirColasDispositivos(void);

int cantidadDispositivos(void);

#endif /* INTERFAZ_H_ */
