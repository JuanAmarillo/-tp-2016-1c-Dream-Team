/*
 * analizador.h
 *
 *  Created on: 23/4/2016
 *      Author: utnso
 */

#ifndef ANALIZADOR_H_
#define ANALIZADOR_H_

t_puntero definirVariable(t_nombre_variable variable);
t_puntero obtenerPosicionVariable(t_nombre_variable variable);
t_valor_variable dereferenciar(t_puntero puntero);

void asignar(t_puntero puntero, t_valor_variable variable);
void imprimir(t_valor_variable valor);
void imprimirTexto(char* texto);
void testParser();

// Configuracion del analizador
static const int CONTENIDO_VARIABLE = 20;
static const int POSICION_MEMORIA = 0x10;
static const char* DEFINICION_VARIABLES = "variables a, b, c";
static const char* ASIGNACION = "a = b + 12";
static const char* IMPRIMIR = "print b";
static const char* IMPRIMIR_TEXTO = "textPrint foo\n";

AnSISOP_funciones functions = {
		.AnSISOP_definirVariable		= definirVariable,
		.AnSISOP_obtenerPosicionVariable= obtenerPosicionVariable,
		.AnSISOP_dereferenciar			= dereferenciar,
		.AnSISOP_asignar				= asignar,
		.AnSISOP_imprimir				= imprimir,
		.AnSISOP_imprimirTexto			= imprimirTexto,

};
AnSISOP_kernel kernel_functions = { };

#endif /* ANALIZADOR_H_ */
