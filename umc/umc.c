#include "umc.h"
#include "../cpu/protocolo_mensaje.h"

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

void enviarProgramaAlSWAP(unsigned pid, unsigned paginasSolicitadas,
		unsigned tamanioCodigo, char* codigoPrograma) {
	t_mensaje codigo;
	unsigned parametrosParaEnviar[1];
	unsigned byte;
	//Enviar programa al SWAP
	codigo->head.codigo = SAVE_PROGRAM;
	codigo->head.cantidad_parametros = 1;
	parametrosParaEnviar[0] = pid;
	codigo->head.tam_extra = paginasSolicitadas * infoMemoria.tamanioDeMarcos;
	for (byte = 0; byte < infoMemoria.tamanioDeMarcos * paginasSolicitadas;
			byte++) {
		if (byte < tamanioCodigo)
			codigo->mensaje_extra[byte] = codigoPrograma[byte];
		else
			codigo->mensaje_extra[byte] = '\0';
	}
	enviarMensaje(clienteSWAP, codigo);
}

void enviarCodigoAlSwap(unsigned paginasSolicitadas,char* codigoPrograma,unsigned pid,unsigned tamanioCodigo)
{
	unsigned respuesta;
	//Reservar espacio en el SWAP
	pedirReservaDeEspacio(pid, paginasSolicitadas);

	//fijarse si pudo reservar
	recv(clienteSWAP,&respuesta,4,0);
	if(respuesta == NOT_ENOUGH_SPACE)
		perror("El SWAP no tiene espacio disponible para almacenar el nuevo programa");
		//El programa se puede albergar en el SWAP
	//Enviar programa al SWAP
	enviarProgramaAlSWAP(pid, paginasSolicitadas, tamanioCodigo, codigoPrograma);
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

void finPrograma(unsigned pid)
{
	t_mensaje finalizarProg;
	finalizarProg.head.codigo = END_PROGRAM;
	finalizarProg.head.cantidad_parametros = 1;
	unsigned parametros[1];
	parametros[0] = pid;
	finalizarProg.parametros = parametros;
	finalizarProg.head.tam_extra = 0;
	finalizarProg.mensaje_extra = NULL;
	if(eliminarDeMemoria(pid) == 0)
	{
		enviarMensaje(clienteSWAP,finalizarProg);
	}
	return;
}

void enviarPaginaAlSWAP(unsigned pagina,void* codigoDelMarco)
{
	t_mensaje aEnviar;
	aEnviar.head.codigo = SAVE_PAGE;
	unsigned parametros[2];
	parametros[0] = procesoActivo;
	parametros[1] = pagina;
	aEnviar.head.cantidad_parametros = 2;
	aEnviar.head.tam_extra = infoMemoria.tamanioDeMarcos;
	aEnviar.parametros = parametros;
	aEnviar.mensaje_extra = codigoDelMarco;
	enviarMensaje(clienteSWAP,aEnviar);
	free(aEnviar);
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

void pedirPagAlSWAP(unsigned pagina) {
	//Pedimos pagina al SWAP
	t_mensaje aEnviar;
	aEnviar.head.codigo = BRING_PAGE_TO_UMC;
	unsigned parametros[2];
	parametros[0] = procesoActivo;
	parametros[1] = pagina;
	aEnviar.head.cantidad_parametros = 2;
	aEnviar.parametros = parametros;
	aEnviar.mensaje_extra = NULL;
	aEnviar.head.tam_extra = 0;
	enviarMensaje(clienteSWAP, aEnviar);
}

void traerPaginaAMemoria(unsigned pagina)
{
	t_mensaje aRecibir;

	//Pedimos pagina al SWAP
	pedirPagAlSWAP(pagina);
	//Recibimos pagina del SWAP

	recibirMensaje(clienteSWAP, &aRecibir);
	if(aRecibir.head.codigo == SWAP_SENDS_PAGE)
		perror("No hay espacio suficiente"); //modificar
// ACA SOLO PUEDO DEVOLVER ESTO, YA TENGO EL ESPACIO RESERVADO PARFA ESA PAGINA
	algoritmoClock(aRecibir.mensaje_extra,pagina);


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
		case FIN_PROG:  finPrograma(mensaje.parametros[0]);
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
