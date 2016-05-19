/*
 * FUNCIONES ANALIZADOR
 */

t_puntero definirVariable(t_nombre_variable variable) {
	printf("definir la variable %c\n", variable);
	return POSICION_MEMORIA;
}

t_puntero obtenerPosicionVariable(t_nombre_variable variable) {
	printf("Obtener posicion de %c\n", variable);
	return POSICION_MEMORIA;
}

t_valor_variable dereferenciar(t_puntero puntero) {
	printf("Dereferenciar %d y su valor es: %d\n", puntero, CONTENIDO_VARIABLE);
	return CONTENIDO_VARIABLE;
}

void asignar(t_puntero puntero, t_valor_variable variable) {
	printf("Asignando en %d el valor %d\n", puntero, variable);
}

// t_valor_variable obtenerValorCompartida(t_nombre_compartida variable);
// t_valor_variable asignarValorCompartida(t_nombre_compartida variable, t_valor_variable valor);
// t_puntero_instruccion irAlLabel(t_nombre_etiqueta etiqueta);
// t_puntero_instruccion llamarFuncion(t_nombre_etiqueta etiqueta, t_posicion donde_retornar, t_puntero_instruccion linea_en_ejecucion);
// t_puntero_instruccion retornar(t_valor_variable retorno);

void imprimir(t_valor_variable valor) {
	printf("Imprimir %d\n", valor);
}

void imprimirTexto(char* texto) {
	printf("ImprimirTexto: %s", texto);
}

// void entradaSalida(t_nombre_dispositivo dispositivo, int tiempo);
// void wait(t_nombre_semaforo identificador_semaforo);
// void signal(t_nombre_semaforo identificador_semaforo);
