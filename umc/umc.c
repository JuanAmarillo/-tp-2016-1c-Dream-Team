#include "umc.h"



void leerArchivoConfig()
{

	t_config *config = config_create("config.conf");

	if (config == NULL) {
		log_error(logger, "Error al leer archivo config.conf\n");
		free(config);
		abort();
	}

	infoConfig.ip = config_get_string_value(config, "IP_SWAP");
	infoConfig.puertoUMC = config_get_string_value(config, "PUERTO");
	infoConfig.puertoSWAP = config_get_string_value(config, "PUERTO_SWAP");
	infoConfig.algoritmo = config_get_string_value(config, "ALGORITMO");
	infoMemoria.marcos = config_get_int_value(config, "MARCOS");
	infoMemoria.tamanioDeMarcos = config_get_int_value(config, "MARCO_SIZE");
	infoMemoria.maxMarcosPorPrograma = config_get_int_value(config,"MARCO_X_PROC");
	infoMemoria.entradasTLB = config_get_int_value(config,"ENTRADAS_TLB");

	free(config->path);
	free(config);
	return;
}

struct sockaddr_in setDireccionUMC()
{
	struct sockaddr_in direccion;
	direccion.sin_family = AF_INET;
	direccion.sin_addr.s_addr = INADDR_ANY;
	direccion.sin_port = htons (atoi(infoConfig.puertoUMC));
	memset(&(direccion.sin_zero), '\0', 8);

	return direccion;
}
struct sockaddr_in setDireccionSWAP()
{
	struct sockaddr_in direccion;
	direccion.sin_family = AF_INET;
	direccion.sin_addr.s_addr = inet_addr(infoConfig.ip);
	direccion.sin_port = htons (atoi(infoConfig.puertoSWAP));
	memset(&(direccion.sin_zero), '\0', 8);

	return direccion;
}

void inicializarEstructuras()
{
	logger = log_create("UMC_TEST.txt", "UMC", 1, LOG_LEVEL_TRACE);
	memoriaPrincipal = malloc(infoMemoria.marcos * infoMemoria.tamanioDeMarcos);
	TLB = list_create();
	tablasDePaginas  = list_create();
	punteroClock = 0;
	return;
}

void clienteDesconectado(int clienteUMC)
{

	pthread_mutex_lock(&mutexClientes);
	FD_CLR(clienteUMC, &master);
	pthread_mutex_unlock(&mutexClientes);

	close(clienteUMC);
	//pthreadexit fijate!
	return;
}

void pedirReservaDeEspacio(unsigned pid,unsigned paginasSolicitadas) {
	//Reservar espacio en el SWAP
	t_mensaje reserva;
	unsigned parametrosParaReservar [2];
	reserva.head.codigo = RESERVE_SPACE;
	reserva.head.cantidad_parametros = 2;
	parametrosParaReservar[0] = pid;
	parametrosParaReservar[1] = paginasSolicitadas;
	reserva.head.tam_extra = 0;
	reserva.mensaje_extra = NULL;
	reserva.parametros = parametrosParaReservar;
	enviarMensaje(clienteSWAP,reserva);
}

void empaquetarYEnviar(t_mensaje mensaje,int clienteUMC)
{
	void *mensaje_empaquetado = empaquetar_mensaje(mensaje);
	unsigned tamanio_mensaje = sizeof(unsigned)*3 + sizeof(unsigned) * mensaje.head.cantidad_parametros + mensaje.head.tam_extra;

	send(clienteUMC, mensaje_empaquetado, tamanio_mensaje, MSG_NOSIGNAL);

	freeMensaje(mensaje_empaquetado);

	return;
}

void enviarProgramaAlSWAP(unsigned pid, unsigned paginasSolicitadas,
		unsigned tamanioCodigo, char* codigoPrograma) {
	t_mensaje codigo;
	unsigned parametrosParaEnviar[1];
	unsigned byte;
	//Enviar programa al SWAP
	codigo.head.codigo = SAVE_PROGRAM;
	codigo.head.cantidad_parametros = 1;
	parametrosParaEnviar[0] = pid;
	codigo.parametros = parametrosParaEnviar;
	codigo.head.tam_extra = paginasSolicitadas * infoMemoria.tamanioDeMarcos;
	for (byte = 0; byte < infoMemoria.tamanioDeMarcos * paginasSolicitadas;
			byte++) {
		if (byte < tamanioCodigo)
			codigo.mensaje_extra[byte] = codigoPrograma[byte];
		else
			codigo.mensaje_extra[byte] = '\0';
	}
	enviarMensaje(clienteSWAP, codigo);
}

void enviarNoHaySuficienteEspacio(int clienteUMC)
{
	t_mensaje noEspacio;
	noEspacio.head.codigo = ALMACENAR_FAILED;
	noEspacio.head.cantidad_parametros = 1;
	noEspacio.head.tam_extra = 0;

	empaquetarYEnviar(noEspacio,clienteUMC);
}

void enviarCodigoAlSwap(unsigned paginasSolicitadas,char* codigoPrograma,unsigned pid,unsigned tamanioCodigo,int clienteUMC)
{
	unsigned respuesta;

	//Reservar espacio en el SWAP
	pedirReservaDeEspacio(pid, paginasSolicitadas);

	//fijarse si pudo reservar
	recv(clienteSWAP,&respuesta,4,0);
	if(respuesta == NOT_ENOUGH_SPACE)
		 enviarNoHaySuficienteEspacio(clienteUMC);

	//Enviar programa al SWAP
	enviarProgramaAlSWAP(pid, paginasSolicitadas, tamanioCodigo, codigoPrograma);
}

void crearTablaDePaginas(unsigned pid,unsigned paginasSolicitadas)
{
	log_trace(logger,"Se procede a crear una tabla de paginas para el proceso %d, con %d paginas\n",pid,paginasSolicitadas);
	int pagina;
	t_tablaDePaginas *tablaPaginas = malloc(sizeof(t_tablaDePaginas));
	tablaPaginas->pid = pid;
	tablaPaginas->entradaTablaPaginas = calloc(paginasSolicitadas,sizeof(t_entradaTablaPaginas));

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

void borrarEntradasTLBSegun(unsigned pidActivo)
{
	unsigned cantidadEntradas = infoMemoria.entradasTLB;
	unsigned entrada;
	t_entradaTLB *entradaTLB;
	pthread_mutex_lock(&mutexTLB);
	for(entrada = 0 ; entrada < cantidadEntradas ; entrada++)
	{
		entradaTLB = list_get(TLB,entrada);
		if(entradaTLB->pid == pidActivo)
			list_remove(TLB,entrada);
	}
	pthread_mutex_unlock(&mutexTLB);

	return;
}
unsigned cambioProcesoActivo(unsigned pid,int clienteUMC, unsigned pidActivo)
{
	if(pidActivo > 0)
		borrarEntradasTLBSegun(pidActivo);

	return pid;
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
		log_error(logger, "Paginas solicitadas mayor al maximo permitido, paginas:%d , maximo:%d\n", paginasSolicitadas, infoMemoria.maxMarcosPorPrograma);
		enviarNoHaySuficienteEspacio(clienteUMC);
		return;
	}
	crearTablaDePaginas(pid,paginasSolicitadas);

	//enviarCodigoAlSwap(paginasSolicitadas,codigoPrograma,pid,tamanioCodigo,clienteUMC); sacar comentario cuando termines de testear
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

void finPrograma(t_mensaje finalizarProg)
{
	unsigned pid = finalizarProg.parametros[0];
	if(eliminarDeMemoria(pid) == 0)
	{
		enviarMensaje(clienteSWAP,finalizarProg);
	}
	return;
}

void enviarPaginaAlSWAP(unsigned pagina,void* codigoDelMarco,unsigned pidActivo)
{
	t_mensaje aEnviar;
	aEnviar.head.codigo = SAVE_PAGE;
	unsigned parametros[2];
	parametros[0] = pidActivo;
	parametros[1] = pagina;
	aEnviar.head.cantidad_parametros = 2;
	aEnviar.head.tam_extra = infoMemoria.tamanioDeMarcos;
	aEnviar.parametros = parametros;
	aEnviar.mensaje_extra = codigoDelMarco;
	enviarMensaje(clienteSWAP,aEnviar);
	return;
}


void llevarMarcoAlSwap(unsigned paginaBuscada,unsigned indice, t_tablaDePaginas *tablaBuscada,unsigned pidActivo)
{
	void* codigoDelMarco = NULL;
	//Pongo el bit de presencia en 0
	tablaBuscada->entradaTablaPaginas[paginaBuscada].estaEnMemoria = 0;
	list_replace(tablasDePaginas,indice,tablaBuscada);
	pthread_mutex_unlock(&mutexTablaPaginas);

	//Llevo el codigo que esta en el marco al SWAP
	pthread_mutex_lock(&mutexMemoria);
	memcpy(codigoDelMarco, memoriaPrincipal+infoMemoria.tamanioDeMarcos*punteroClock, infoMemoria.tamanioDeMarcos);
	pthread_mutex_unlock(&mutexMemoria);
	enviarPaginaAlSWAP(paginaBuscada,codigoDelMarco,pidActivo);
	return;
}

//Busca Bit presencia = 0 && bit Modificado = 0
int buscaNoPresenciaNoModificado()
{
	t_tablaDePaginas* tablaBuscada;
	unsigned indice;
	unsigned cantidadDePaginas;
	unsigned paginaBuscada;
	punteroClock=0;

	for(indice=0;indice < list_size(tablasDePaginas);indice++)
	{
		tablaBuscada = list_get(tablasDePaginas,indice);
		cantidadDePaginas = sizeof(tablaBuscada)/sizeof(t_tablaDePaginas);

		for(paginaBuscada = 0; paginaBuscada < cantidadDePaginas; paginaBuscada++)
		{
			if(tablaBuscada->entradaTablaPaginas[paginaBuscada].estaEnMemoria == 0)
				if(tablaBuscada->entradaTablaPaginas[paginaBuscada].fueModificado==0)
						return 1;
			punteroClock++;
		}
	}
	return 0;
}

//Busca Bit presencia = 0 && bit Modificado = 1
int buscaNoPresenciaSiModificado(unsigned pidActivo)
{
	t_tablaDePaginas* tablaBuscada;
	unsigned indice;
	unsigned cantidadDePaginas;
	unsigned paginaBuscada;
	punteroClock=0;

	for(indice=0;indice < list_size(tablasDePaginas);indice++)
	{
		cantidadDePaginas = sizeof(tablaBuscada)/sizeof(t_tablaDePaginas);

		for(paginaBuscada = 0; paginaBuscada < cantidadDePaginas; paginaBuscada++)
		{
			tablaBuscada = list_get(tablasDePaginas,indice);
			if(tablaBuscada->entradaTablaPaginas[paginaBuscada].estaEnMemoria == 0)
				if(tablaBuscada->entradaTablaPaginas[paginaBuscada].fueModificado==1)
					return 1;

			if(tablaBuscada->entradaTablaPaginas[paginaBuscada].estaEnMemoria == 1)
				llevarMarcoAlSwap(paginaBuscada,indice,tablaBuscada,pidActivo);
			punteroClock++;
		}
	}
	return 0;
}
void falloDePaginaMejorado(unsigned pidActivo)
{
	while(1)
	{
		if(buscaNoPresenciaNoModificado() == 1)
			return;
		if(buscaNoPresenciaSiModificado(pidActivo) ==1)
			return;
	}
	return;
}

void falloDePagina(unsigned pidActivo)
{
	t_tablaDePaginas* tablaBuscada;
	unsigned indice;
	unsigned paginaBuscada;
	unsigned cantidadDePaginas;

	for(indice=0;indice < list_size(tablasDePaginas);indice++)
	{
		tablaBuscada = list_get(tablasDePaginas,indice);
		cantidadDePaginas = sizeof(tablaBuscada)/sizeof(t_tablaDePaginas);

		for(paginaBuscada = 0; paginaBuscada < cantidadDePaginas; paginaBuscada++)
		{
			//Busca si la pagina esta en memoria
			if(tablaBuscada->entradaTablaPaginas[paginaBuscada].estaEnMemoria == 1)
				if(tablaBuscada->entradaTablaPaginas[paginaBuscada].marco == punteroClock)
					llevarMarcoAlSwap(paginaBuscada,indice,tablaBuscada,pidActivo);
		}

	}
	return;
}


void actualizarPagina(unsigned pagina,unsigned pidActivo)
{
	t_tablaDePaginas *tablaDePagina;
	unsigned indice;
	for(indice = 0; indice < list_size(tablasDePaginas);indice++)
	{
		tablaDePagina = list_get(tablasDePaginas,indice);
		if(tablaDePagina->pid == pidActivo)
		{
			tablaDePagina->entradaTablaPaginas[pagina].estaEnMemoria = 1;
			tablaDePagina->entradaTablaPaginas[pagina].marco = punteroClock;
		}
	}
	return;
}

void escribirEnMemoria(void* codigoPrograma,unsigned tamanioPrograma, unsigned pagina,unsigned pidActivo)
{
	actualizarPagina(pagina,pidActivo);
	pthread_mutex_lock(&mutexMemoria);
	memcpy(memoriaPrincipal + infoMemoria.tamanioDeMarcos*punteroClock,codigoPrograma,tamanioPrograma);
	pthread_mutex_unlock(&mutexMemoria);

	return;
}

void algoritmoClock(void* codigoPrograma,unsigned tamanioPrograma,unsigned pagina,unsigned pidActivo)
{
	pthread_mutex_lock(&mutexClock);

	//Eleccion entre Algoritmos
	if(strcmp("CLOCK",infoConfig.algoritmo))
		falloDePagina(pidActivo);
	if(strcmp("CLOCKMEJORADO",infoConfig.algoritmo))
		falloDePaginaMejorado(pidActivo);

	//Escribe en memoria la nueva pagina que mando el SWAP
	escribirEnMemoria(codigoPrograma,tamanioPrograma,pagina,pidActivo);

	//Se fija si el punteroClock llego al ultimo marco
	if(punteroClock < infoMemoria.marcos)
		punteroClock++;
	else
		punteroClock = 0;

	pthread_mutex_unlock(&mutexClock);
	return;
}

void pedirPagAlSWAP(unsigned pagina,unsigned pidActual) {
	//Pedimos pagina al SWAP
	t_mensaje aEnviar;
	aEnviar.head.codigo = BRING_PAGE_TO_UMC;
	unsigned parametros[2];
	parametros[0] = pidActual;
	parametros[1] = pagina;
	aEnviar.head.cantidad_parametros = 2;
	aEnviar.parametros = parametros;
	aEnviar.mensaje_extra = NULL;
	aEnviar.head.tam_extra = 0;
	enviarMensaje(clienteSWAP, aEnviar);
}

void traerPaginaAMemoria(unsigned pagina,unsigned pidActual)
{
	t_mensaje aRecibir;

	//Pedimos pagina al SWAP
	pedirPagAlSWAP(pagina,pidActual);

	//Recibimos pagina del SWAP
	recibirMensaje(clienteSWAP, &aRecibir);
	algoritmoClock(aRecibir.mensaje_extra,aRecibir.head.tam_extra,pagina,pidActual);


	return;
}

void actualizarTLB(t_entradaTablaPaginas entradaDePaginas,unsigned pagina,unsigned pidActual)
{
	//LRU
	int tamanioMaximoTLB = infoMemoria.entradasTLB;
	int tamanioTLB = list_size(TLB);

	t_entradaTLB *entradaTLB = malloc(sizeof(entradaTLB));
	entradaTLB->pid = pidActual;
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

int buscarEnTLB(unsigned paginaBuscada,unsigned pidActual)
{
	int indice;
	int marco;
	t_entradaTLB *entradaTLB;
	for(indice=0;indice < infoMemoria.entradasTLB;indice++)
	{
		entradaTLB = list_get(TLB,indice);
		if(entradaTLB[indice].pid == pidActual && entradaTLB[indice].pagina == paginaBuscada)
		  {
			marco = entradaTLB[indice].marco;

			//Sacar la entrada y ponerla inicio de la lista
			list_remove(TLB,indice);
			list_add_in_index(TLB,0,entradaTLB);
			return marco;
		  }
	}
	return -1;
}

void traducirPaginaAMarco(unsigned pagina,int *marco,unsigned pidActual)
{

	unsigned indice;
	pthread_mutex_lock(&mutexTablaPaginas);  //cuidado con los mutex boludo!
	unsigned cantidadDeTablas = list_size(tablasDePaginas);
	t_tablaDePaginas *tablaDePaginas;

	//Buscar en TLB
	*marco = buscarEnTLB(pagina,pidActual);
	if(*marco != -1)
		return;

	//Buscar en tabla de paginas
	for(indice = 0;indice < cantidadDeTablas; indice++)
	{
		tablaDePaginas= list_get(tablasDePaginas,indice);

		if(tablaDePaginas->pid == pidActual)
		{

			if(tablaDePaginas->entradaTablaPaginas[pagina].estaEnMemoria == 1)
			{
				//Esta en memoria se copia el marco
				*marco = tablaDePaginas->entradaTablaPaginas[pagina].marco;
				pthread_mutex_unlock(&mutexTablaPaginas);
				actualizarTLB(tablaDePaginas->entradaTablaPaginas[pagina],pagina,pidActual);
				return;
			}
			else
			{	// Buscar en swap
				traerPaginaAMemoria(pagina,pidActual);
				*marco = tablaDePaginas->entradaTablaPaginas[pagina].marco;
				pthread_mutex_unlock(&mutexTablaPaginas);
				actualizarTLB(tablaDePaginas->entradaTablaPaginas[pagina],pagina,pidActual);
			}
		}

	}
	return;
}

void almacenarBytesEnPagina(t_mensaje mensaje,unsigned pidActivo)
{
	unsigned pagina  = mensaje.parametros[0];
	unsigned offset  = mensaje.parametros[1];
	unsigned tamanio = mensaje.parametros[2];

	int marco;
	traducirPaginaAMarco(pagina,&marco,pidActivo);
	memcpy(memoriaPrincipal+infoMemoria.tamanioDeMarcos*marco+offset,(void*)mensaje.parametros[3],tamanio);

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

void enviarBytesDeUnaPagina(t_mensaje mensaje,int clienteUMC,unsigned pidActual)
{
	unsigned pagina  = mensaje.parametros[0];
	unsigned offset  = mensaje.parametros[1];
	unsigned tamanio = mensaje.parametros[2];
	void* codigoAEnviar = malloc(tamanio);
	int marco;
	traducirPaginaAMarco(pagina,&marco,pidActual);

	memcpy(codigoAEnviar,memoriaPrincipal+infoMemoria.tamanioDeMarcos*marco+offset,tamanio);

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

void accionSegunCabecera(int clienteUMC,unsigned pid)
{
	unsigned pidActivo = pid;
	int cabeceraDelMensaje;
	t_mensaje mensaje;

	while(1){
		if(recibirMensaje(clienteUMC,&mensaje) <= 0)
			clienteDesconectado(clienteUMC);
		desempaquetar_mensaje(&mensaje);
		cabeceraDelMensaje = mensaje.head.codigo;

		switch(cabeceraDelMensaje){
			case INIT_PROG: inicializarPrograma(mensaje,clienteUMC);
				break;
			case FIN_PROG:  finPrograma(mensaje);
				break;
			case GET_DATA:  enviarBytesDeUnaPagina(mensaje,clienteUMC,pidActivo);
				break;
			case GET_TAM_PAGINA: enviarTamanioDePagina(clienteUMC);
				break;
			case CAMBIO_PROCESO:
				pidActivo = cambioProcesoActivo(mensaje.parametros[0],clienteUMC,pidActivo);
				break;
			case RECORD_DATA: almacenarBytesEnPagina(mensaje,pidActivo);
				break;
		}
		freeMensaje(&mensaje);
	}
	return;
}

void* gestionarSolicitudesDeOperacion(int clienteUMC)
{
	//Hago esto porque no se como pasarle varios parametros a un hilo
	accionSegunCabecera(clienteUMC,0);
	return NULL;
}

int recibirConexiones()
{

	direccionServidorUMC = setDireccionUMC();

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
	direccionServidorSWAP = setDireccionSWAP();
	clienteSWAP = socket(AF_INET, SOCK_STREAM, 0);
		if (connect(clienteSWAP, (void*) &direccionServidorSWAP, sizeof(direccionServidorSWAP)) != 0) {
			log_error(logger,"La UMC fallo al conectarse al SWAP");
			abort();
		}
		log_trace(logger,"La UMC se conecto con exito al SWAP");
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
	pthread_mutex_init(&mutexClock,NULL);
	pthread_mutex_init(&mutexTLB,NULL);

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
				log_trace(logger,"Se crea un hilo");
				pthread_create(&cliente,NULL,gestionarSolicitudesDeOperacion,clienteUMC);
			}
		}
	}

	return;
}
/*
int main(){


	//Config
	leerArchivoConfig();
	inicializarEstructuras();
	conectarAlSWAP();

	//servidor
	gestionarConexiones();

	return 0;
}
/*

   //Pruebas commons
	t_list *hola = list_create();
	t_tablaDePaginas *tabla = malloc(sizeof(t_tablaDePaginas));
	t_tablaDePaginas *tabla2 = malloc(sizeof(t_tablaDePaginas));
	t_tablaDePaginas *tabla3 = malloc(sizeof(t_tablaDePaginas));
	t_tablaDePaginas *aux;
	tabla3->entradaTablaPaginas = calloc(2,sizeof(tabla->entradaTablaPaginas));
	tabla3->pid = 4;
	tabla2->pid = 3;
	tabla->pid = 2;
	list_add(hola,tabla);
	list_add(hola,tabla2);
	list_add(hola,tabla3);
	list_remove(hola,1);
	//printf("%d",list_size(hola));
	aux = list_get(hola,0);
	printf("%d",aux->pid);
	aux = list_get(hola,1);
	printf("%d",aux->pid);
	unsigned cantidad = list_size(hola);
	printf("%d",aux->pid);



	return 0;

}*/
