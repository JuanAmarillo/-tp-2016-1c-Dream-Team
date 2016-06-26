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

t_puntero parser_definirVariable(t_nombre_variable identificador_variable);
t_puntero parser_obtenerPosicionVariable(t_nombre_variable identificador_variable);
t_valor_variable parser_dereferenciar(t_puntero direccion_variable);
void parser_asignar(t_puntero direccion_variable, t_valor_variable valor);
t_valor_variable parser_obtenerValorCompartida(t_nombre_compartida variable);
t_valor_variable parser_asignarValorCompartida(t_nombre_compartida variable, t_valor_variable valor);
void parser_irAlLabel(t_nombre_etiqueta etiqueta);
void parser_llamarSinRetorno(t_nombre_etiqueta etiqueta);
void parser_llamarConRetorno(t_nombre_etiqueta etiqueta, t_puntero donde_retornar);
void parser_finalizar();
void parser_retornar(t_valor_variable retorno);
void parser_imprimir(t_valor_variable valor_mostrar);
void parser_imprimirTexto(char* texto);
void parser_entradaSalida(t_nombre_dispositivo dispositivo, int tiempo);
void parser_wait(t_nombre_semaforo identificador_semaforo);
void parser_signal(t_nombre_semaforo identificador_semaforo);

AnSISOP_funciones functions = {
		.AnSISOP_definirVariable		= parser_definirVariable,
		.AnSISOP_obtenerPosicionVariable= parser_obtenerPosicionVariable,
		.AnSISOP_dereferenciar			= parser_dereferenciar,
		.AnSISOP_asignar				= parser_asignar,
		/*.AnSISOP_obtenerValorCompartida	= parser_obtenerValorCompartida,
		.AnSISOP_asignarValorCompartida	= parser_asignarValorCompartida,
		.AnSISOP_irAlLabel				= parser_irAlLabel,
		.AnSISOP_llamarSinRetorno		= parser_llamarSinRetorno,*/
		.AnSISOP_llamarConRetorno		= parser_llamarConRetorno,
		/*.AnSISOP_finalizar				= parser_finalizar,
		.AnSISOP_retornar				= parser_retornar,
		.AnSISOP_imprimir				= parser_imprimir,
		.AnSISOP_imprimirTexto			= parser_imprimirTexto,
		.AnSISOP_entradaSalida			= parser_entradaSalida,*/

};
AnSISOP_kernel kernel_functions = {
		/*.AnSISOP_wait					= parser_wait,
		.AnSISOP_signal					= parser_signal,*/
};

#endif /* ANALIZADOR_H_ */
