#ifndef PROTOCOLO_MENSAJE_H_
#define PROTOCOLO_MENSAJE_H_

/*
 * Estructuras de datos
 */
typedef struct {
  unsigned codigo;
  unsigned cantidad_parametros;
  unsigned tam_extra;
} mensajeHead_t;

typedef struct {
  mensajeHead_t head;
  unsigned *parametros;
  char *mensaje_extra;
} mensaje_t;

/*
 * Funciones / Procedimientos
 */
void *empaquetar_mensaje(mensaje_t);
mensajeHead_t desempaquetar_head(const void *);
mensaje_t desempaquetar_mensaje(const void *);
int enviarMensaje(int, mensaje_t);
int recibirMensaje(int, mensaje_t *);
int recibirBytes(int, void *, unsigned);
void testMensajeProtocolo();

#endif /* PROTOCOLO_MENSAJE_H_ */
