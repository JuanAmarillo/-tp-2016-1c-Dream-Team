/*
 * variables_compartidas.h
 *
 *  Created on: 25/7/2016
 *      Author: utnso
 */

#ifndef VARIABLES_COMPARTIDAS_H_
#define VARIABLES_COMPARTIDAS_H_

typedef struct
{
	char *nombre;
	int valor;
}t_variable_compartida;

t_variable_compartida *variables_compartidas;
unsigned int cantidad_variables_compartidas;

#endif /* VARIABLES_COMPARTIDAS_H_ */
