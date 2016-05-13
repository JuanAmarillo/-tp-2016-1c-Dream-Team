/*
 * protocolo_mensaje.c
 *
 *  Created on: 13/5/2016
 *      Author: utnso
 */
/*
 * Hipotesis: Al trabajar todos con la misma maquina virtual, no tenemos en cuenta la variacion de tamaño de los tipo de datos segun las arquitecturas.
 */

/*
 * empaquetar_mensaje();
 * Parametros:
 *		-> mensaje: Bloque de mensaje a empaquetar
 * Descripcion: Dado un mensaje, lo empaqueta para enviarlo
 * Return: *mensaje_empaquetado
 */
void *empaquetar_mensaje(mensaje_t mensaje) {
	unsigned desplazamiento = 0;

	// Creo un bloque en memoria con la tamaño del Head + Payload
	char *mensaje_empaquetado = malloc(sizeof(unsigned)*3 + sizeof(unsigned) * mensaje.cantidad_parametros + mensaje.tam_extra);

	// Copio el head en el bloque de memoria creado
	memcpy(mensaje_empaquetado + desplazamiento, &mensaje.codigo, sizeof(unsigned));
	desplazamiento += sizeof(unsigned);
	memcpy(mensaje_empaquetado + desplazamiento, &mensaje.cantidad_parametros, sizeof(unsigned));
	desplazamiento += sizeof(unsigned);
	memcpy(mensaje_empaquetado + desplazamiento, &mensaje.tam_extra, sizeof(unsigned));
	desplazamiento += sizeof(unsigned);

	// Copio los parametros
	unsigned i_parametro;
	for (i_parametro = 0; i_parametro < mensaje.cantidad_parametros; i_parametro++){
		memcpy(mensaje_empaquetado + desplazamiento, &mensaje.parametros[i_parametro], sizeof(unsigned));
		desplazamiento += sizeof(unsigned);
	}

	// Copio el mensaje_extra
	memcpy(mensaje_empaquetado + desplazamiento, mensaje.mensaje_extra, mensaje.tam_extra);

	// Devuelvo
	return mensaje_empaquetado;
}

/*
 * desempaquetar_mensaje();
 * Parametros:
 *		-> buffer: Bytes a desempaquetar
 * Descripcion: Dado un buffer, lo desempaqueta
 * Return: mensaje_t mensaje_desempaquetado
 */
mensaje_t desempaquetar_mensaje(const void *buffer) {
	unsigned desplazamiento = 0;
	mensaje_t mensaje_desempaquetado;

	// Desempaqueto el HEAD
	memcpy(&mensaje_desempaquetado.codigo, buffer, sizeof(unsigned));
	desplazamiento += sizeof(unsigned);
	memcpy(&mensaje_desempaquetado.cantidad_parametros, buffer + desplazamiento, sizeof(unsigned));
	desplazamiento += sizeof(unsigned);
	memcpy(&mensaje_desempaquetado.tam_extra, buffer + desplazamiento, sizeof(unsigned));
	desplazamiento += sizeof(unsigned);

	// Creo memoria para los parametros
	mensaje_desempaquetado.parametros = malloc(sizeof(unsigned) * mensaje_desempaquetado.cantidad_parametros);

	// Desempaqueto los parametros
	unsigned i_parametro;
	for (i_parametro = 0; i_parametro < mensaje_desempaquetado.cantidad_parametros; i_parametro++){
		memcpy(&mensaje_desempaquetado.parametros[i_parametro], buffer + desplazamiento, sizeof(unsigned));
		desplazamiento += sizeof(unsigned);
	}

	// Desempaqueto lo extra
	mensaje_desempaquetado.mensaje_extra = malloc(mensaje_desempaquetado.tam_extra);
	memcpy(mensaje_desempaquetado.mensaje_extra, buffer + desplazamiento, mensaje_desempaquetado.tam_extra);

	return mensaje_desempaquetado;
}


void testMensajeProtocolo(){
	mensaje_t mensaje;
	mensaje_t mensaje_new;
	unsigned parametros[2];

	// Defino el mensaje a empaquetar
	mensaje.codigo = 20;
	mensaje.cantidad_parametros = 2;
	mensaje.tam_extra = 0;
	mensaje.mensaje_extra = NULL;
	mensaje.parametros = parametros;
	parametros[0] = 3;
	parametros[1] = 7;

	//
	const char *buffer = empaquetar_mensaje(mensaje);

	mensaje_new = desempaquetar_mensaje(buffer);

	printf("%i",mensaje_new.codigo);
}
