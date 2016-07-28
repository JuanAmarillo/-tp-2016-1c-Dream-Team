#ifndef NUCLEO_13_INTERFAZ_H_
#define NUCLEO_13_INTERFAZ_H_

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
void imprimirListaProcesosBloq(const t_list *lista);
void imprimirColasDispositivos(void);
void mostrarCompartidas(void);

int cantidadDispositivos(void);

#endif /* NUCLEO_13_INTERFAZ_H_ */
