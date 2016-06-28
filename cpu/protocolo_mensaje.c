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
void *empaquetar_mensaje(t_mensaje mensaje) {

	// Variables usadas
	unsigned desplazamiento = 0;
	unsigned i_parametro;

	// Creo un bloque en memoria con la tamaño del Head + Payload
	void *mensaje_empaquetado = malloc(sizeof(t_mensajeHead) + sizeof(unsigned) * mensaje.head.cantidad_parametros + mensaje.head.tam_extra);

	// Copio el head en el bloque de memoria creado
	memcpy(mensaje_empaquetado + desplazamiento, &mensaje.head.codigo, sizeof(unsigned));
	desplazamiento += sizeof(unsigned);
	memcpy(mensaje_empaquetado + desplazamiento, &mensaje.head.cantidad_parametros, sizeof(unsigned));
	desplazamiento += sizeof(unsigned);
	memcpy(mensaje_empaquetado + desplazamiento, &mensaje.head.tam_extra, sizeof(unsigned));
	desplazamiento += sizeof(unsigned);

	// Copio los parametros
	for (i_parametro = 0; i_parametro < mensaje.head.cantidad_parametros; i_parametro++){
		memcpy(mensaje_empaquetado + desplazamiento, &mensaje.parametros[i_parametro], sizeof(unsigned));
		desplazamiento += sizeof(unsigned);
	}

	// Copio el mensaje_extra
	memcpy(mensaje_empaquetado + desplazamiento, mensaje.mensaje_extra, mensaje.head.tam_extra);

	// Devuelvo
	return mensaje_empaquetado;
}

/*
 * desempaquetar_head();
 * Parametros:
 *		-> buffer: Bytes a desempaquetar
 * Descripcion: Dado un buffer, desempaqueta el HEAD
 * Return: t_mensajeHead mensaje_head
 */
t_mensajeHead desempaquetar_head(const void *buffer) {

	// Declaro variables usadas
	unsigned desplazamiento = 0;
	t_mensajeHead mensaje_head;

	// Desempaqueto el head
	memcpy(&mensaje_head.codigo, buffer, sizeof(unsigned));
	desplazamiento += sizeof(unsigned);
	memcpy(&mensaje_head.cantidad_parametros, buffer + desplazamiento, sizeof(unsigned));
	desplazamiento += sizeof(unsigned);
	memcpy(&mensaje_head.tam_extra, buffer + desplazamiento, sizeof(unsigned));

	return mensaje_head;
}

/*
 * desempaquetar_mensaje();
 * Parametros:
 *		-> buffer: Bytes a desempaquetar
 * Descripcion: Dado un buffer, lo desempaqueta
 * Return: t_mensaje mensaje_desempaquetado
 */
t_mensaje desempaquetar_mensaje(const void *buffer) {

	// Declaro variables usadas
	unsigned i_parametro;
	unsigned desplazamiento = 0;
	t_mensaje mensaje_desempaquetado;

	// Desempaqueto el HEAD
	mensaje_desempaquetado.head = desempaquetar_head(buffer);
	desplazamiento = sizeof(t_mensajeHead);

	// Creo memoria para los parametros
	mensaje_desempaquetado.parametros = malloc(sizeof(unsigned) * mensaje_desempaquetado.head.cantidad_parametros);

	// Desempaqueto los parametros
	for (i_parametro = 0; i_parametro < mensaje_desempaquetado.head.cantidad_parametros; i_parametro++){
		memcpy(&mensaje_desempaquetado.parametros[i_parametro], buffer + desplazamiento, sizeof(unsigned));
		desplazamiento += sizeof(unsigned);
	}

	// Desempaqueto lo extra
	mensaje_desempaquetado.mensaje_extra = malloc(mensaje_desempaquetado.head.tam_extra);
	memcpy(mensaje_desempaquetado.mensaje_extra, buffer + desplazamiento, mensaje_desempaquetado.head.tam_extra);

	return mensaje_desempaquetado;
}

/*
 * enviarMensaje();
 * Parametros:
 * 	-> serverSocket :: ID del socket donde voy a enviar el mensaje
 * 	-> mensaje	:: Mensaje a enviar
 * Descripcion: Envia un mensaje a traves serverSocket
 * Return:
 * 		-> -1 :: Error
 * 		->  other :: -
 */
int enviarMensaje(int serverSocket, t_mensaje mensaje){

	void *mensaje_empaquetado = empaquetar_mensaje(mensaje);
	unsigned tamano_mensaje = sizeof(unsigned)*3 + sizeof(unsigned) * mensaje.head.cantidad_parametros + mensaje.head.tam_extra;

	int enviar = send(serverSocket, mensaje_empaquetado, tamano_mensaje, MSG_NOSIGNAL);

	free(mensaje_empaquetado);

	return enviar;

}

/*
 * recibirMensaje();
 * Parametros:
 * 		-> serverSocket :: ID del socket desde donde voy a recibir el mensaje
 * 		-> mensaje :: Donde voy a guardar el mensaje
 * 		-> tamano :: Tamaño que ocupa el mensaje
 * Descripcion: Recibe un mensaje del serverSocket y lo guarda en mensaje
 * Return:
 * 		-> -1 :: Error
 * 		->  other :: -
 */
int recibirMensaje(int serverSocket, t_mensaje *mensaje){

	// Declaro variables
	t_mensajeHead mensaje_head;
	unsigned desplazamiento = 0;
	int recibir;

	// Recibo el HEAD
	void *buffer_head = malloc(sizeof(unsigned)*3);
	recibir = recibirBytes(serverSocket, buffer_head, sizeof(unsigned)*3);
	if (recibir <= 0){
		free(buffer_head);
		return recibir;
	}

	// Obtengo los valores del HEAD
	mensaje_head = desempaquetar_head(buffer_head);
	desplazamiento += sizeof(t_mensajeHead);

	// Me preparo para recibir el Payload
	unsigned faltan_recibir = sizeof(unsigned) * mensaje_head.cantidad_parametros + mensaje_head.tam_extra;
	void *bufferTotal = malloc(sizeof(t_mensajeHead) + faltan_recibir);
	memcpy(bufferTotal, buffer_head, sizeof(t_mensajeHead));

	// Recibo el Payload
	if(faltan_recibir > 0) recibir = recibirBytes(serverSocket, bufferTotal + desplazamiento, faltan_recibir);

	// Desempaqueto el mensaje
	*mensaje = desempaquetar_mensaje(bufferTotal);

	// Limpieza
	free(buffer_head);
	free(bufferTotal);

	//
	return recibir;
}

/*
 * recibirBytes();
 * Parametros:
 * 		-> serverSocket :: ID del socket desde donde voy a recibir el mensaje
 * 		-> buffer :: Donde voy a guardar el mensaje
 * 		-> tamano :: Tamaño que ocupa el mensaje
 * Descripcion: Recibe un mensaje del serverSocket y lo guarda en buffer
 * Return:
 * 		-> -1 :: Error
 * 		->  other :: -
 */

int recibirBytes(int serverSocket, void *buffer, unsigned tamano){
	int recibir = recv(serverSocket, buffer, tamano, MSG_WAITALL);
	return recibir;
}

/*
 * freeMensaje();
 * Parametros:
 * 		-> mensaje :: Puntero a mensaje a destruir
 * Descripcion: Procedimiento que libera memoria de un mensaje
 * Return: -
 */

void freeMensaje(t_mensaje *mensaje) {
  free(mensaje->parametros);
  free(mensaje->mensaje_extra);
}

void testMensajeProtocolo(){
	// Declaro variables usadas
	t_mensaje mensaje;
	t_mensaje mensaje_new;
	unsigned parametros[2];

	// Defino el mensaje a empaquetar
	mensaje.head.codigo = 20;
	mensaje.head.cantidad_parametros = 2;
	mensaje.head.tam_extra = 0;
	mensaje.mensaje_extra = NULL;
	mensaje.parametros = parametros;
	parametros[0] = 3;
	parametros[1] = 7;

	//
	const char *buffer = empaquetar_mensaje(mensaje);

	mensaje_new = desempaquetar_mensaje(buffer);

	printf("%i",mensaje_new.head.codigo);
}
