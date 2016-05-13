/*
 * protocolo_mensaje.h
 *
 *  Created on: 13/5/2016
 *      Author: utnso
 */

#ifndef PROTOCOLO_MENSAJE_H_
#define PROTOCOLO_MENSAJE_H_

/*
 * Estructuras de datos
 */
typedef struct {
  unsigned codigo;
  unsigned cantidad_parametros;
  unsigned tam_extra;
  unsigned *parametros;
  char *mensaje_extra;
} mensaje_t;

/*
 * Funciones / Procedimientos
 */
void *empaquetar_mensaje(mensaje_t);
mensaje_t desempaquetar_mensaje(const void *);
void testMensajeProtocolo();

#endif /* PROTOCOLO_MENSAJE_H_ */
