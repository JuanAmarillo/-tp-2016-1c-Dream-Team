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
	infoMemoria.marcos = config_get_int_value(config, "MARCOS");
	infoMemoria.tamanioDeMarcos = config_get_int_value(config, "MARCO_SIZE");
	infoMemoria.maxMarcosPorPrograma = config_get_int_value(config,"MARCO_X_PROC");
	infoMemoria.entradasTLB = config_get_int_value(config,"ENTRADAS_TLB");

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
	TLB = list_create();
	tablasDePaginas  = list_create();
	punteroClock = 0;
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

void clienteDesconectado(int clienteUMC)
{
	perror("El cliente se desconecto\n");

	pthread_mutex_lock(&mutexClientes);
	FD_CLR(clienteUMC, &master);
	pthread_mutex_unlock(&mutexClientes);

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
void enviarCodigoAlSwap(unsigned paginasSolicitadas,char* codigoPrograma,unsigned pid,unsigned tamanioCodigo)
{
	unsigned byte;
	unsigned pagina;

	char buffer[infoMemoria.tamanioDeMarcos];

	//Reservar espacio en el SWAP
	enviar(RESERVE_SPACE,clienteSWAP);
	enviar(pid,clienteSWAP);
	enviar(paginasSolicitadas,clienteSWAP);

	//fijarse si pudo reservar

	//Enviar programa al SWAP
	enviar(SAVE_PROGRAM);
	enviar(pid);
	for(pagina=0;pagina<paginasSolicitadas;pagina++)
	{
		for(byte=0;byte<infoMemoria.tamanioDeMarcos;byte++)
		{
			if(byte+pagina*infoMemoria.tamanioDeMarcos != tamanioCodigo)
				buffer[byte] = codigoPrograma[byte + pagina*infoMemoria.tamanioDeMarcos];

			else
			{
				buffer[byte] = codigoPrograma[byte + pagina*infoMemoria.tamanioDeMarcos];
				enviar(buffer,clienteSWAP);
				return;
			}
		}
		//Envia la pagina con el codigo correspondiente
		enviar(buffer,clienteSWAP);
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

	pthread_mutex_lock(&mutexTablaPaginas);
	list_add(tablasDePaginas,tablaPaginas);
	pthread_mutex_unlock(&mutexTablaPaginas);
	return;

}
void freeMensaje(t_mensaje *mensaje) {
  free(mensaje->parametros);
  free(mensaje->mensaje_extra);
  return;
}
void cambioProcesoActivo(unsigned pid)
{
	pthread_mutex_lock(&mutexProcesoActivo);
	procesoActivo = pid;
	pthread_mutex_unlock(&mutexProcesoActivo);
	//borrar cache
	return;
}


void inicializarPrograma(t_mensaje mensaje,int clienteUMC)
{
	unsigned pid = mensaje.parametros[0];
	unsigned paginasSolicitadas = mensaje.parametros[1];
	char*codigoPrograma = malloc(paginasSolicitadas*infoMemoria.tamanioDeMarcos);
	codigoPrograma = mensaje.mensaje_extra;
	unsigned tamanioCodigo=mensaje.head.tam_extra;

	if(paginasSolicitadas > infoMemoria.maxMarcosPorPrograma)
	{
		//enviar un error: paginas solicitadas excede el maximo
		return;
	}

	crearTablaDePaginas(pid,paginasSolicitadas);
	enviarCodigoAlSwap(paginasSolicitadas,codigoPrograma,pid,tamanioCodigo);
	free(codigoPrograma);
	return;
}
int eliminarDeMemoria(unsigned pid)
{
	t_tablaDePaginas *buscador;
	int index;

	//tabla de paginas

	pthread_mutex_lock(&mutexTablaPaginas);
	for(index=0;index < list_size(tablasDePaginas); index++ )
	{
		buscador = list_get(tablasDePaginas,index);
		if(buscador->pid == pid)
		{
			list_remove(tablasDePaginas,index);
			pthread_mutex_unlock(&mutexTablaPaginas);
			return 1;
		}
	}
	pthread_mutex_unlock(&mutexTablaPaginas);

	return 0;
}

void finPrograma(t_mensaje mensaje)
{
	unsigned pid = mensaje.parametros[0];
	if(eliminarDeMemoria(pid) == 0)
	{
		enviar(END_PROGRAM,clienteSWAP);
		enviar(pid,clienteSWAP);
	}
	return;
}
void enviarPaginaAlSWAP(unsigned pagina,void* codigoDelMarco)
{
	enviar(SAVE_PAGE,clienteSWAP);
	enviar(procesoActivo,clienteSWAP);
	enviar(pagina,clienteSWAP);
	enviar(codigoDelMarco,clienteSWAP);

	return;
}
void falloDePagina()
{
	t_tablaDePaginas* tablaBuscada;
	unsigned indice;
	unsigned paginaBuscada;
	unsigned cantidadDePaginas;
	void* codigoDelMarco;
	for(indice=0;indice < list_size(tablasDePaginas);indice++)
	{
		tablaBuscada = list_get(tablasDePaginas,indice);
		cantidadDePaginas = sizeof(tablaBuscada)/sizeof(t_tablaDePaginas);

		for(paginaBuscada = 0; paginaBuscada < cantidadDePaginas; paginaBuscada++)
		{
			//Busca si la pagina esta en memoria
			if(tablaBuscada->entradaTablaPaginas[paginaBuscada].estaEnMemoria == 1)
				if(tablaBuscada->entradaTablaPaginas[paginaBuscada].marco == punteroClock)
				{
					//Pongo el bit de presencia en 0
					tablaBuscada->entradaTablaPaginas[paginaBuscada].estaEnMemoria = 0;
					list_replace(tablasDePaginas,indice,tablaBuscada);

					//Llevo el codigo que esta en el marco al SWAP
					pthread_mutex_lock(&mutexMemoria);
					memcpy(codigoDelMarco, memoriaPrincipal+infoMemoria.tamanioDeMarcos*punteroClock, sizeof(infoMemoria.tamanioDeMarcos));
					pthread_mutex_unlock(&mutexMemoria);
					enviarPaginaAlSWAP(paginaBuscada,codigoDelMarco);
					return;
				}
		}
	}
	return;
}

void actualizarPagina(unsigned pagina)
{
	t_tablaDePaginas *tablaDePagina;
	unsigned indice;
	for(indice = 0; indice < list_size(tablasDePaginas);indice++)
	{
		tablaDePagina = list_get(tablasDePaginas,indice);
		if(tablaDePagina->pid == procesoActivo)
		{
			tablaDePagina->entradaTablaPaginas[pagina].estaEnMemoria = 1;
			tablaDePagina->entradaTablaPaginas[pagina].marco = punteroClock;
		}
	}
	return;
}
void escribirEnMemoria(void* codigoPrograma, unsigned pagina)
{
	actualizarPagina(pagina);
	memcpy(memoriaPrincipal + infoMemoria.tamanioDeMarcos*punteroClock,codigoPrograma,sizeof(codigoPrograma));
}

void algoritmoClock(void* codigoPrograma,unsigned pagina)
{
	pthread_mutex_lock(&mutexClock);
	falloDePagina();
	escribirEnMemoria(codigoPrograma,pagina);
	punteroClock++;
	pthread_mutex_unlock(&mutexClock);
	return;
}

void traerPaginaAMemoria(unsigned pagina)
{
	unsigned codigo;
	void* codigoPrograma;


	//Pedimos pagina al SWAP
	enviar(BRING_PAGE_TO_UMC,clienteSWAP);
	enviar(procesoActivo,clienteSWAP);
	enviar(pagina,clienteSWAP);

	//Recibimos pagina del SWAP
	recibir(&codigo,4,clienteSWAP);
	if(codigo == SWAP_SENDS_PAGE)
		recibir(codigoPrograma,clienteSWAP);
	else
		perror("No hay espacio suficiente"); //modificar

	algoritmoClock(codigoPrograma,pagina);


	return;
}
void actualizarTLB(t_entradaTablaPaginas entradaDePaginas,unsigned pagina)
{
	//LRU
	int tamanioMaximoTLB = infoMemoria.entradasTLB;
	int tamanioTLB = list_size(TLB);

	t_entradaTLB *entradaTLB = malloc(sizeof(entradaTLB));
	entradaTLB->pagina = pagina;
	entradaTLB->estaEnMemoria = entradaDePaginas.estaEnMemoria;
	entradaTLB->fueModificado = entradaDePaginas.fueModificado;
	entradaTLB->marco         = entradaDePaginas.marco;

	if(tamanioTLB == tamanioMaximoTLB)
		list_replace(TLB,tamanioTLB-1,entradaTLB);
	else
		list_add_in_index(TLB,tamanioTLB,entradaTLB);

	return;
}
int buscarEnTLB(unsigned paginaBuscada)
{
	int indice;
	int marco;
	t_entradaTLB *entradaTLB;
	for(indice=0;indice < infoMemoria.entradasTLB;indice++)
	{
		entradaTLB = list_get(TLB,indice);
		if(TLB[indice].pagina == paginaBuscada)
		  {
			marco = TLB[indice].marco;

			//Sacar la entrada y ponerla inicio de la lista
			list_remove(TLB,indice);
			list_add_in_index(TLB,0,entradaTLB);
			return marco;
		  }
	}
	return -1;
}

void traducirPaginaAMarco(unsigned pagina,int *marco)
{
	unsigned indice;
	pthread_mutex_lock(&mutexTablaPaginas);
	unsigned cantidadDeTablas = list_size(tablasDePaginas);
	t_tablaDePaginas *tablaDePaginas;

	//Buscar en TLB
	*marco = buscarEnTLB(pagina);
	if(*marco != -1)
		return;

	//Buscar en tabla de paginas
	pthread_mutex_lock(&mutexProcesoActivo);
	for(indice = 0;indice < cantidadDeTablas; indice++)
	{
		tablaDePaginas= list_get(tablasDePaginas,indice);

		if(tablaDePaginas->pid == procesoActivo)
		{
			*marco = tablaDePaginas->entradaTablaPaginas[pagina].marco;
			pthread_mutex_unlock(&mutexTablaPaginas);
			pthread_mutex_unlock(&mutexProcesoActivo);
			actualizarTLB(tablaDePaginas->entradaTablaPaginas[pagina],pagina);

			return;
		}
	}
	pthread_mutex_unlock(&mutexTablaPaginas);
	// Buscar en swap
	traerPaginaAMemoria(pagina);
	traducirPaginaAMarco(pagina,marco);
	pthread_mutex_unlock(&mutexProcesoActivo);
	return;
}

void almacenarBytesEnPagina(t_mensaje mensaje)
{
	unsigned pagina  = mensaje.parametros[0];
	unsigned offset  = mensaje.parametros[1];
	unsigned tamanio = mensaje.parametros[2];
	void*    buffer  = mensaje.mensaje_extra;
	int marco;
	traducirPaginaAMarco(pagina,&marco);
	memcpy(memoriaPrincipal+infoMemoria.tamanioDeMarcos*marco+offset,buffer,sizeof(buffer));

	return;
}
void empaquetarYEnviar(t_mensaje mensaje,int clienteUMC)
{
	void *mensaje_empaquetado = empaquetar_mensaje(mensaje);
	unsigned tamanio_mensaje = sizeof(unsigned)*3 + sizeof(unsigned) * mensaje.head.cantidad_parametros + mensaje.head.tam_extra;

	send(clienteUMC, mensaje_empaquetado, tamanio_mensaje, MSG_NOSIGNAL);

	freeMensaje(mensaje_empaquetado);

	return;
}

void enviarCodigoAlCPU(char* codigoAEnviar, unsigned tamanio,int clienteUMC)
{
	t_mensaje mensaje;
	mensaje.head.codigo = RETURN_DATA;
	mensaje.head.cantidad_parametros = 0;
	mensaje.head.tam_extra = tamanio;
	mensaje.parametros = NULL;
	mensaje.mensaje_extra = codigoAEnviar;
	empaquetarYEnviar(mensaje,clienteUMC);

	return;
}
void enviarBytesDeUnaPagina(t_mensaje mensaje,int clienteUMC)
{
	unsigned pagina  = mensaje.parametros[0];
	unsigned offset  = mensaje.parametros[1];
	unsigned tamanio = mensaje.parametros[2];
	char* codigoAEnviar = malloc(tamanio);
	unsigned tamanioRecorrido;
	int marco;
	traducirPaginaAMarco(pagina,&marco);

	pthread_mutex_lock(&mutexMemoria);
	char* memoria = (char*)memoriaPrincipal;
	pthread_mutex_unlock(&mutexMemoria);

	for(tamanioRecorrido=0;tamanioRecorrido < tamanio;tamanioRecorrido++ )
	{
		codigoAEnviar[tamanioRecorrido] = memoria[infoMemoria.tamanioDeMarcos*marco+offset+tamanioRecorrido];
	}
	enviarCodigoAlCPU(codigoAEnviar,tamanio,clienteUMC);
	free(codigoAEnviar);

	return;
}
void enviarTamanioDePagina(int clienteUMC)
{
	t_mensaje mensaje;
	mensaje.head.codigo = RETURN_TAM_PAGINA;
	mensaje.head.cantidad_parametros = 1;
	mensaje.head.tam_extra = 0;
	mensaje.parametros[0] = infoMemoria.tamanioDeMarcos;
	mensaje.mensaje_extra = NULL;
	empaquetarYEnviar(mensaje,clienteUMC);

	return;
}

void accionSegunCabecera(int cabeceraDelMensaje,t_mensaje mensaje,int clienteUMC)
{
	switch(cabeceraDelMensaje){
		case INIT_PROG: inicializarPrograma(mensaje,clienteUMC);
			break;
		case FIN_PROG:  finPrograma(mensaje);
			break;
		case GET_DATA:  enviarBytesDeUnaPagina(mensaje,clienteUMC);
			break;
		case GET_TAM_PAGINA: enviarTamanioDePagina(clienteUMC);
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

int recibir(void *buffer,unsigned tamanioDelMensaje,int clienteUMC)
{
	int bytesRecibidos = recv(clienteUMC, buffer, tamanioDelMensaje, 0);
	if (bytesRecibidos <= 0) {
		perror("El cliente se desconecto\n");
		close(clienteUMC);
		pthread_mutex_lock(&mutexClientes);
		FD_CLR(clienteUMC, &master);
		pthread_mutex_unlock(&mutexClientes);
		return 0;
	}
	return bytesRecibidos;
}

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
void enviar(void *buffer,int cliente)
{
	send(cliente, buffer, sizeof(buffer), 0);
	free(buffer);
	return;
}

void gestionarConexiones()
{

	fd_set fdsParaLectura;
	int maximoFD;
	int fdBuscador;
	pthread_t cliente;
	int clienteUMC;
	pthread_mutex_init(&mutexClientes,NULL);
	pthread_mutex_init(&mutexMemoria,NULL);
	pthread_mutex_init(&mutexTablaPaginas,NULL);
	pthread_mutex_init(&mutexProcesoActivo,NULL);

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

/*
   //Pruebas commons
	t_list *hola = list_create();
	t_tablaDePaginas *tabla = malloc(sizeof(t_tablaDePaginas));
	t_tablaDePaginas *tabla2 = malloc(sizeof(t_tablaDePaginas));
	t_tablaDePaginas *tabla3 = malloc(sizeof(t_tablaDePaginas));
	t_tablaDePaginas *aux;
	unsigned cantidad;
	tabla3->pid = 4;
	tabla2->pid = 3;
	tabla->pid = 2;
	list_add(hola,tabla);
	list_add_in_index(hola,0,tabla2);
	list_add_in_index(hola,1,tabla3);
	list_remove(hola,1);
	//printf("%d",list_size(hola));
	aux = list_get(hola,0);
	printf("%d",aux->pid);
	aux = list_get(hola,1);
	printf("%d",aux->pid);
	cantidad = list_size(hola);
	printf("%d",aux->pid);
*/





	return 0;
}


