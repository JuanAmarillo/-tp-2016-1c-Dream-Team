/*
 * variables_compartidas.h
 *
 *  Created on: 25/7/2016
 *      Author: utnso
 */

#ifndef VARIABLES_COMPARTIDAS_H_
#define VARIABLES_COMPARTIDAS_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "archivoLog.h"

typedef struct
{
	char *nombre;
	int valor;
}t_variable_compartida;

t_variable_compartida *variables_compartidas;
unsigned int cantidad_variables_compartidas;

int obtenerValorCompartida(const char *nombreVariable);
int existeVariable(const char *nombreVariable);
void asignarCompartida(const char *nombre, int nuevoValor);

#endif /* VARIABLES_COMPARTIDAS_H_ */
