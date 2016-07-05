#ifndef PROTOCOLO_MENSAJE_H_
#define PROTOCOLO_MENSAJE_H_

#include <commons/string.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <signal.h>
#include "protocolo_mensaje.h"
/*
 * Estructuras de datos
 */
typedef struct {
  unsigned codigo;
  unsigned cantidad_parametros;
  unsigned tam_extra;
} t_mensajeHead;

typedef struct {
  t_mensajeHead head;
  unsigned *parametros;
  char *mensaje_extra;
} t_mensaje;

/*
 * Funciones / Procedimientos
 */
void *empaquetar_mensaje(t_mensaje);
t_mensajeHead desempaquetar_head(const void *);
t_mensaje desempaquetar_mensaje(const void *);
int enviarMensaje(int, t_mensaje);
int recibirMensaje(int, t_mensaje *);
int recibirBytes(int, void *, unsigned);
void testMensajeProtocolo();
void freeMensaje(t_mensaje *mensaje);

#endif /* PROTOCOLO_MENSAJE_H_ */
