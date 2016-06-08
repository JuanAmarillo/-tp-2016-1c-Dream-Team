#include "umc.h"


void leerArchivoConfig()
{

	t_config *config = config_create("config.conf");

	if (config == NULL) {
		free(config);
		abort();
	}

	infoConfig.ip = config_get_string_value(config, "IP");
	infoConfig.puertoUMC = config_get_string_value(config, "PUERTO");
	infoConfig.puertoSWAP = config_get_string_value(config, "PUERTO_SWAP");
	infoMemoria.marcos = config_get_string_value(config, "MARCOS");
	infoMemoria.tamanioDeMarcos = config_get_string_value(config, "MARCO_SIZE");
	infoMemoria.maxMarcosPorPrograma = config_get_string_value(config,"MARCO_X_PROC");

	free(config->path);
	free(config);
	return;
}

struct sockaddr_in setDireccion(const char *puerto)
{
	struct sockaddr_in direccion;
	direccion.sin_family = AF_INET;
	direccion.sin_addr.s_addr = inet_addr(infoConfig.ip);
	direccion.sin_port = htons (atoi(puerto));
	memset(&(direccion.sin_zero), '\0', 8);

	return direccion;
}

void inicializarEstructuras()
{
	memoriaPrincipal = malloc(infoMemoria.marcos * infoMemoria.tamanioDeMarcos);
	tablasDePaginas  = list_create();
	return;
}

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
void clienteDesconectado(int clienteUMC)
{
	perror("El cliente se desconecto\n");

	pthread_mutex_lock(&mutex);
	FD_CLR(clienteUMC, &master);
	pthread_mutex_unlock(&mutex);

	close(clienteUMC);

    // hacer un pthread_exit !! <--

	return;
}
void recibirMensaje(int clienteUMC, t_mensaje *mensaje){

	// Declaro variables
	t_mensajeHead mensaje_head;
	int recibir;


	// Recibo el HEAD
	void *buffer_head = malloc(sizeof(unsigned)*3);
	recibir = recv(clienteUMC, buffer_head, sizeof(unsigned)*3, MSG_WAITALL);
	if (recibir <= 0){
		free(buffer_head);
		clienteDesconectado(clienteUMC);
		return;
	}

	// Obtengo los valores del HEAD
	mensaje_head = desempaquetar_head(buffer_head);

	// Me preparo para recibir el Payload
	unsigned faltan_recibir = sizeof(unsigned) * mensaje_head.cantidad_parametros + mensaje_head.tam_extra;
	void *bufferTotal = malloc(sizeof(t_mensajeHead) + faltan_recibir);
	memcpy(bufferTotal, buffer_head, sizeof(t_mensajeHead));

	// Recibo el Payload
	recibir = recv(clienteUMC, bufferTotal + sizeof(t_mensajeHead), faltan_recibir,MSG_WAITALL);
	if(recibir <= 0){
		free(buffer_head);
		free(bufferTotal);
		clienteDesconectado(clienteUMC);
		return;
	}

	// Desempaqueto el mensaje
	*mensaje = desempaquetar_mensaje(bufferTotal);

	// Limpieza
	free(buffer_head);
	free(bufferTotal);

	//
	return;
}
void enviarCodigoAlSwap(unsigned paginasSolicitadas,char* codigoPrograma,unsigned pid)
{
	unsigned byte;
	unsigned pagina;

	char buffer[infoMemoria.tamanioDeMarcos];

	//Reservar espacio en el SWAP
	enviarDatos(RESERVE_SPACE);
	enviarDatos(pid);
	enviarDatos(paginasSolicitadas);

	//fijarse si pudo reservar

	//Enviar programa al SWAP
	enviarDatos(SAVE_PROGRAM);
	enviarDatos(pid);
	for(pagina=0;pagina<paginasSolicitadas;pagina++)
	{
		for(byte=0;byte<infoMemoria.tamanioDeMarcos;byte++)
		{
			if(codigoPrograma[byte+pagina*infoMemoria.tamanioDeMarcos] != '\0')
				buffer[byte] = codigoPrograma[byte + pagina*infoMemoria.tamanioDeMarcos];

			else
			{
				enviarDatos(buffer);
				return;
			}
		}
		//Envia la pagina con el codigo correspondiente
		enviarDatos(buffer);
		memset(buffer, '\0', infoMemoria.tamanioDeMarcos);


	}
	return;
}

void crearTablaDePaginas(unsigned pid,unsigned paginasSolicitadas)
{
	int pagina;
	t_tablaDePaginas *tablaPaginas = malloc(sizeof(t_tablaDePaginas));
	tablaPaginas->pid = pid;
	tablaPaginas->entradaTablaPaginas = calloc(paginasSolicitadas,sizeof(tablaPaginas->entradaTablaPaginas));

	for(pagina=0;pagina < paginasSolicitadas; pagina++)
	{
		tablaPaginas->entradaTablaPaginas[pagina].estaEnMemoria = 0;
		tablaPaginas->entradaTablaPaginas[pagina].fueModificado = 0;
	}

	pthread_mutex_lock(&mutex);
	list_add(tablasDePaginas,tablaPaginas);
	pthread_mutex_unlock(&mutex);
	return;

}

void inicializarPrograma(t_mensaje mensaje,int clienteUMC)
{
	unsigned pid = mensaje.parametros[0];
	unsigned paginasSolicitadas = mensaje.parametros[1];
	char *codigoPrograma = mensaje.parametros[2];// creo que no solo en el 2 fijarse !!!! !! !!!!!!!!!!!!!!!!!

	if(paginasSolicitadas > infoMemoria.maxMarcosPorPrograma)
	{
		//enviar un error: paginas solicitadas excede el maximo
		return;
	}

	crearTablaDePaginas(pid,paginasSolicitadas);
	enviarCodigoAlSwap(paginasSolicitadas,codigoPrograma,pid);

	pthread_mutex_lock(&mutex);
	procesoActivo = pid;
	pthread_mutex_unlock(&mutex);

	return;
}

void finPrograma(t_mensaje mensaje)
{
	unsigned pid = mensaje.parametros[0];
	//if(estaEnMemoria(pid)
	//  borrar las paginas de la lista de paginas
	//else
	enviarDatos(END_PROGRAM);
	enviarDatos(pid);

	return;
}

void almacenarBytesEnPagina(t_mensaje mensaje)
{
	unsigned pagina  = mensaje.parametros[0];
	unsigned offset  = mensaje.parametros[1];
	unsigned tamanio = mensaje.parametros[2];
	char*    buffer  = mensaje.parametros[3];
	//1.traer con el getlist y el pdi del proceso actual la tabla correspondiente
	//2. fijarse la pagina y el offset y meter el buffer con el tam correspondiente
	//3. si no esta en la memoria, preguntar al swap, traerlo y repetir 1 y 2

	return;
}

void enviarBytesDeUnaPagina(t_mensaje mensaje,int clienteUMC)
{
	unsigned pagina  = mensaje.parametros[0];
	unsigned offset  = mensaje.parametros[1];
	unsigned tamanio = mensaje.parametros[2];
	//1.traer con el gestlist y el pdi actual la tabla correspondiente
	//2.poner en una variable los bytes segun la pagina offset y tamaño
	//3.enviar al cpu
	//4.si no esta en memoria preguntar al swap,traerlo y repetir 1,2,3

	return;
}

void accionSegunCabecera(int cabeceraDelMensaje,t_mensaje mensaje,int clienteUMC)
{
	switch(cabeceraDelMensaje){
		case INIT_PROG: inicializarPrograma(mensaje,clienteUMC);
			break;
		case FIN_PROG:  finPrograma(mensaje);
			break;
		case GET_DATA: ; enviarBytesDeUnaPagina(mensaje,clienteUMC);
			break;
		case GET_TAM_PAGINA: ;
			break;
		case RESERVE_MEMORY: ;
			break;
		case RECORD_DATA: almacenarBytesEnPagina(mensaje);
			break;
	}
	return;
}


void gestionarSolicitudesDeOperacion(int clienteUMC)
{
	t_mensaje mensaje;
	recibirMensaje(clienteUMC,&mensaje);
	accionSegunCabecera(mensaje.head.codigo,mensaje,clienteUMC);

	return;
}
int recibirConexiones()
{

	direccionServidorUMC = setDireccion(infoConfig.puertoUMC);

	servidorUMC = socket(AF_INET, SOCK_STREAM, 0);

	int activado = 1;
	setsockopt(servidorUMC, SOL_SOCKET, SO_REUSEADDR, &activado, sizeof(activado)); //para reutilizar dirreciones

	if (bind(servidorUMC, (void*) &direccionServidorUMC, sizeof(direccionServidorUMC)) != 0) {
		perror("Falló asociando el puerto\n");
		abort();
	}

	printf("Estoy escuchando\n");
	listen(servidorUMC, 100);
	FD_SET(servidorUMC,&master);

	return servidorUMC;
}

int aceptarConexion()
{
	int clienteUMC;
	struct sockaddr_in direccion;  //del cpu o del nucleo
	unsigned int tamanioDireccion;
	clienteUMC = accept(servidorUMC, (void*) &direccion, &tamanioDireccion);
	return clienteUMC ;
}
/*
int recibirDatos(void *buffer,unsigned tamanioDelMensaje,int clienteUMC)
{
	int bytesRecibidos = recv(clienteUMC, buffer, tamanioDelMensaje, 0);
	if (bytesRecibidos <= 0) {
		perror("El cliente se desconecto\n");
		close(clienteUMC);
		FD_CLR(clienteUMC, &master);
		return 0;
	}
	printf("UMC: El mensaje recibido es: %s\n", buffer);
	return bytesRecibidos;
}*/

void conectarAlSWAP()
{
	direccionServidorSWAP = setDireccion(infoConfig.puertoSWAP);
	clienteSWAP = socket(AF_INET, SOCK_STREAM, 0);
		if (connect(clienteSWAP, (void*) &direccionServidorSWAP, sizeof(direccionServidorSWAP)) != 0) {
			perror("No se pudo conectar");
			abort();
		}
		return ;
}
void enviarDatos(void* buffer)
{
	send(clienteSWAP, buffer, sizeof(buffer), 0);
	free(buffer);
	return;
}

void gestionarConexiones() // el select basicamente
{

	fd_set fdsParaLectura;
	int maximoFD;
	int fdBuscador;
	pthread_t cliente;
	int clienteUMC;
	pthread_mutex_init(&mutex,NULL);

	FD_ZERO(&master);
	FD_ZERO(&fdsParaLectura);

	maximoFD = recibirConexiones();

	while(1){
		fdsParaLectura = master;

		if (  select(maximoFD+1,&fdsParaLectura,NULL,NULL,NULL) == -1  ){ //comprobar si el select funciona
			perror("Error en el select");
			abort();
		}

		for(fdBuscador=3; fdBuscador <= maximoFD; fdBuscador++) // explora los FDs que estan listos para leer
		{
			if( FD_ISSET(fdBuscador,&fdsParaLectura) ) { //entra una conexion, la acepta y la agrega al master
				if(fdBuscador == servidorUMC)
					clienteUMC = aceptarConexion();
				else{
					FD_SET(clienteUMC, &master);
					if(clienteUMC > maximoFD) //Actualzar el maximo fd
						maximoFD = clienteUMC;
				}

			}
			else{
				pthread_create(&cliente,NULL,(void*)gestionarSolicitudesDeOperacion,clienteUMC);
			}
		}
	}

	return;
}

int main(){

	//Config
	leerArchivoConfig();
	inicializarEstructuras();
	conectarAlSWAP();

	//servidor
	gestionarConexiones();




	return 0;
}


