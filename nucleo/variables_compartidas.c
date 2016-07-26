#include "variables_compartidas.h"

int existeVariable(const char *nombreVariable)
{
	int i;
	for(i = 0; i < cantidad_variables_compartidas; ++i)
		if(!strcmp(nombreVariable, variables_compartidas[i].nombre))
			return 1;
	return 0;
}

int obtenerValorCompartida(const char *nombreVariable)
{
	int i;
	for(i = 0; i < cantidad_variables_compartidas && strcmp(nombreVariable, variables_compartidas[i].nombre); ++i);
	if(i >= cantidad_variables_compartidas)
	{
		perror("Se intento buscar una variable compartida inexistente: abort()");
		abort();
	}
	return variables_compartidas[i].valor;
}

void asignarCompartida(const char *nombre, int nuevoValor)
{
	int i;
	for(i = 0; i < cantidad_variables_compartidas; ++i)
	{
		if(!strcmp(nombre, variables_compartidas[i].nombre))
		{
			variables_compartidas[i].valor = nuevoValor;
			return;
		}
	}
	if(i >= cantidad_variables_compartidas)
	{
		perror("Se intento buscar una variable compartida inexistente: abort()");
		abort();
	}
}
