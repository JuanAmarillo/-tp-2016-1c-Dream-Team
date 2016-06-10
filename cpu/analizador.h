/*
 * analizador.h
 *
 *  Created on: 23/4/2016
 *      Author: utnso
 */

#ifndef ANALIZADOR_H_
#define ANALIZADOR_H_


t_nombre_variable nombreVariable_aBuscar;

int _is_variableX(t_variable *);

t_puntero definirVariable(t_nombre_variable identificador_variable);
t_puntero obtenerPosicionVariable(t_nombre_variable identificador_variable);
t_valor_variable dereferenciar(t_puntero direccion_variable);

void asignar(t_puntero direccion_variable, t_valor_variable valor);
t_valor_variable obtenerValorCompartida(t_nombre_compartida variable);
t_valor_variable asignarValorCompartida(t_nombre_compartida variable, t_valor_variable valor);
void irAlLabel(t_nombre_etiqueta etiqueta);
void llamarSinRetorno(t_nombre_etiqueta etiqueta);
void llamarConRetorno(t_nombre_etiqueta etiqueta, t_puntero donde_retornar);
void finalizar();
void retornar(t_valor_variable retorno);
void imprimir(t_valor_variable valor_mostrar);
void imprimirTexto(char* texto);
void entradaSalida(t_nombre_dispositivo dispositivo, int tiempo);
void wait(t_nombre_semaforo identificador_semaforo);
void signal(t_nombre_semaforo identificador_semaforo);

AnSISOP_funciones functions = {
		.AnSISOP_definirVariable		= definirVariable,
		.AnSISOP_obtenerPosicionVariable= obtenerPosicionVariable,
		.AnSISOP_dereferenciar			= dereferenciar,
		.AnSISOP_asignar				= asignar,
		.AnSISOP_obtenerValorCompartida	= obtenerValorCompartida,
		.AnSISOP_asignarValorCompartida	= asignarValorCompartida,
		.AnSISOP_irAlLabel				= irAlLabel,
		.AnSISOP_llamarSinRetorno		= llamarSinRetorno,
		.AnSISOP_llamarConRetorno		= llamarConRetorno,
		.AnSISOP_finalizar				= finalizar,
		.AnSISOP_retornar				= retornar,
		.AnSISOP_imprimir				= imprimir,
		.AnSISOP_imprimirTexto			= imprimirTexto,
		.AnSISOP_entradaSalida			= entradaSalida,

};
AnSISOP_kernel kernel_functions = {
		.AnSISOP_wait					= wait,
		.AnSISOP_signal					= signal,
};

#endif /* ANALIZADOR_H_ */
