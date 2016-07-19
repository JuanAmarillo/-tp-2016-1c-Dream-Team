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
	infoMemoria.retardo = config_get_int_value(config,"RETARDO");

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
void finalizarUMC()
{
	log_trace(logger,"Se finaliza el UMC ");
	free(memoriaPrincipal);
	list_destroy(TLB);
	list_destroy(tablasDePaginas);
}
void inicializarEstructuras()
{
	logger = log_create("UMC_TEST.txt", "UMC", 1, LOG_LEVEL_TRACE);
	loggerConsola = log_create("UMC_CONSOLA.txt","UMC",1,LOG_LEVEL_TRACE);
	memoriaPrincipal = malloc(infoMemoria.marcos * infoMemoria.tamanioDeMarcos);
	TLB = list_create();
	tablasDePaginas  = list_create();
	return;
}

void clienteDesconectado(int clienteUMC)
{

	pthread_mutex_lock(&mutexClientes);
	FD_CLR(clienteUMC, &master);
	pthread_mutex_unlock(&mutexClientes);

	close(clienteUMC);
	pthread_exit(NULL);
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
	enviarMensaje(clienteUMC, mensaje);
	return;
}

void enviarProgramaAlSWAP(unsigned pid, unsigned paginasSolicitadas, unsigned tamanioCodigo, char* codigoPrograma) {
	log_trace(logger,"INICIO enviarProgramaAlSwap");
	t_mensaje codigo;
	unsigned parametrosParaEnviar[1];

	//Enviar programa al SWAP
	codigo.head.codigo = SAVE_PROGRAM;
	codigo.head.cantidad_parametros = 1;
	parametrosParaEnviar[0] = pid;
	codigo.parametros = parametrosParaEnviar;
	codigo.head.tam_extra = paginasSolicitadas * infoMemoria.tamanioDeMarcos;

	codigo.mensaje_extra = malloc(paginasSolicitadas * infoMemoria.tamanioDeMarcos);;

	log_trace(logger,"%s", codigo.mensaje_extra);
	enviarMensaje(clienteSWAP, codigo);
	free(codigo.mensaje_extra);
	log_trace(logger,"FIN enviarProgramaAlSwap");
}

void enviarSuficienteEspacio(int clienteUMC, int codigo)
{
	log_trace(logger,"Entro a la funcion enviarSuficienteEspacio");
	t_mensaje noEspacio;
	noEspacio.head.codigo = codigo;
	noEspacio.head.cantidad_parametros = 1;
	noEspacio.head.tam_extra = 0;
	empaquetarYEnviar(noEspacio,clienteUMC);
	log_trace(logger,"Salio de la funcion enviarSuficienteEspacio");
}

unsigned enviarCodigoAlSwap(unsigned paginasSolicitadas,char* codigoPrograma,unsigned pid,unsigned tamanioCodigo,int clienteUMC)
{
	log_trace(logger,"Entro a la funcion enviarCodigoAlSwap");
	t_mensaje respuesta;

	//Reservar espacio en el SWAP
	log_trace(logger,"Pide espacio para reservar el programa al SWAP");
	pedirReservaDeEspacio(pid, paginasSolicitadas);

	//fijarse si pudo reservar
	recibirMensaje(clienteSWAP,&respuesta);
	log_trace(logger,"La cabecera recibida es: %d",respuesta.head.codigo);
	if(respuesta.head.codigo == NOT_ENOUGH_SPACE)
	{
		log_trace(logger,"No hay suficiente espacio para almacenar el programa");
		enviarSuficienteEspacio(clienteUMC,ALMACENAR_FAILED);
		return 0;
	}
	//Enviar programa al SWAP
	log_trace(logger,"Hay suficiente espacio, se envia el programa al SWAP");
	enviarProgramaAlSWAP(pid, paginasSolicitadas, tamanioCodigo, codigoPrograma);
	enviarSuficienteEspacio(clienteUMC,ALMACENAR_OK);
	log_trace(logger,"Salio de la funcion enviarCodigoAlSwap");
	return 1;
}

void crearTablaDePaginas(unsigned pid,unsigned paginasSolicitadas)
{
	log_trace(logger,"Entro a la funcion crearTablaDePaginas");
	log_trace(logger,"Se procede a crear una tabla de paginas para el proceso %d, con %d paginas\n",pid,paginasSolicitadas);
	int pagina;
	pthread_mutex_lock(&mutexTablaPaginas);
	t_tablaDePaginas *tablaPaginas = malloc(sizeof(t_tablaDePaginas));
	t_entradaTablaPaginas *entradaTablaPaginas = calloc(paginasSolicitadas,sizeof(t_entradaTablaPaginas));
	tablaPaginas->pid = pid;
	tablaPaginas->punteroClock = 0;
	tablaPaginas->entradaTablaPaginas = entradaTablaPaginas;
	tablaPaginas->cantidadEntradasTablaPagina = paginasSolicitadas;

	if(paginasSolicitadas > infoMemoria.maxMarcosPorPrograma){
		tablaPaginas->paginasEnMemoria = calloc(infoMemoria.maxMarcosPorPrograma,sizeof(int));
		tablaPaginas->cantidadEntradasMemoria = infoMemoria.maxMarcosPorPrograma;
		for(pagina = 0; pagina < infoMemoria.maxMarcosPorPrograma;pagina++)
			tablaPaginas->paginasEnMemoria[pagina]= -1;
	}
	else{
		tablaPaginas->paginasEnMemoria = calloc(paginasSolicitadas,sizeof(int));
		tablaPaginas->cantidadEntradasMemoria = paginasSolicitadas;
		for(pagina=0;pagina < paginasSolicitadas;pagina++)
			tablaPaginas->paginasEnMemoria[pagina] = pagina;

	}
	for(pagina=0;pagina < paginasSolicitadas; pagina++)
	{

		tablaPaginas->entradaTablaPaginas[pagina].estaEnMemoria = 0;
		tablaPaginas->entradaTablaPaginas[pagina].fueModificado = 0;
	}
	log_trace(logger,"\n  Se creo una tabla de Pagina:\n- PID : %d\n- Paginas : %d\n- PunteroClock: %d\n",tablaPaginas->pid,tablaPaginas->cantidadEntradasTablaPagina,tablaPaginas->punteroClock);

	list_add(tablasDePaginas,tablaPaginas);
	pthread_mutex_unlock(&mutexTablaPaginas);
	log_trace(logger,"Salio de  la funcion crearTablaDePaginas");
	return;

}

void borrarEntradasTLBSegun(unsigned pidActivo)
{
	unsigned entrada;
	t_entradaTLB *entradaTLB;
	pthread_mutex_lock(&mutexTLB);
	unsigned cantidadEntradas = list_size(TLB);
	for(entrada = 0 ; entrada < cantidadEntradas ; entrada++)
	{
		entradaTLB = list_get(TLB,entrada);
		if(entradaTLB->pid == pidActivo)
			list_remove(TLB,entrada);
	}
	pthread_mutex_unlock(&mutexTLB);

	return;
}
unsigned cambioProcesoActivo(unsigned pid,unsigned pidActivo)
{
	log_trace(logger,"Se cambia de proceso id por : %d", pid);
	if(pidActivo > 0)
	{
		log_trace(logger,"Se borran las entradas de la TLB del pid anterior");
		borrarEntradasTLBSegun(pidActivo);
	}
	return pid ;
}

void inicializarPrograma(t_mensaje mensaje,int clienteUMC)
{
	log_trace(logger,"Inicio Incializar Programa");
	unsigned pid = mensaje.parametros[0];
	unsigned paginasSolicitadas = mensaje.parametros[1];
	char* codigoPrograma = malloc(paginasSolicitadas*infoMemoria.tamanioDeMarcos);
	memcpy(codigoPrograma, mensaje.mensaje_extra, mensaje.head.tam_extra);
	unsigned tamanioCodigo=mensaje.head.tam_extra;

	if(enviarCodigoAlSwap(paginasSolicitadas,codigoPrograma,pid,tamanioCodigo,clienteUMC) == 1)
		crearTablaDePaginas(pid,paginasSolicitadas);

 	free(codigoPrograma);
 	free(mensaje.mensaje_extra);

 	log_trace(logger,"Fin Inicializar Programa");
	return;

}

void eliminarDeMemoria(unsigned pid)
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
			return;
		}
	}
	return;
}

void finPrograma(t_mensaje finalizarProg)
{
	unsigned pid = finalizarProg.parametros[0];
	log_trace(logger,"Se finaliza el programa con pid:%d ",pid);
	eliminarDeMemoria(pid);
	enviarMensaje(clienteSWAP,finalizarProg);
	return;
}


void falloPagina(t_tablaDePaginas* tablaBuscada,unsigned indice,unsigned pidActivo,unsigned paginaApuntada)
{
	log_trace(logger,"INICIO FALLO PAGINA");
	log_trace(logger,"Se produce un fallo de pagina en la pagina:%d del proceso:%d indice:%d",paginaApuntada,pidActivo,indice);
	void* codigoDelMarco = malloc(infoMemoria.tamanioDeMarcos);
	unsigned marco = tablaBuscada->entradaTablaPaginas[paginaApuntada].marco ;
	t_tablaDePaginas* tablaTest;
	unsigned size = list_size(tablasDePaginas);

	//Pongo el bit de presencia en 0
	tablaBuscada->entradaTablaPaginas[paginaApuntada].estaEnMemoria = 0;
	list_replace(tablasDePaginas,indice,tablaBuscada);
	tablaTest = list_get(tablasDePaginas,indice);
	if(tablaBuscada->entradaTablaPaginas[paginaApuntada].estaEnMemoria == tablaTest->entradaTablaPaginas[paginaApuntada].estaEnMemoria &&
			list_size(tablasDePaginas) == size)
		log_trace(logger,"Se actualizo la pagina:%d del proceso:%d",paginaApuntada,pidActivo);
	else
		log_error(logger,"Hubo un error en actualizar la pagina:%d del proceso:%d",paginaApuntada,pidActivo);


	//Llevo el codigo que esta en el marco al SWAP
	pthread_mutex_lock(&mutexMemoria);
	memcpy(codigoDelMarco, memoriaPrincipal+infoMemoria.tamanioDeMarcos*marco , infoMemoria.tamanioDeMarcos);
	pthread_mutex_unlock(&mutexMemoria);
	log_trace(logger,"Se envia la pagina:%d del proceso:%d al SWAP",paginaApuntada,pidActivo);
	enviarPaginaAlSWAP(paginaApuntada,codigoDelMarco,pidActivo);
	free(codigoDelMarco);
	log_trace(logger,"FIN FALLO PAGINA");
	return;
}

//Busca Bit presencia = 0 && bit Modificado = 0
int buscaNoPresenciaNoModificado(t_tablaDePaginas *tablaBuscada,unsigned *punteroClock,unsigned pidActivo)
{
	unsigned paginaApuntada;
	int pagina;
	for(pagina = 0 ; pagina  < tablaBuscada->cantidadEntradasMemoria;pagina++ )
	{
		paginaApuntada = tablaBuscada->paginasEnMemoria[*punteroClock];
		if(tablaBuscada->entradaTablaPaginas[paginaApuntada].estaEnMemoria == 0)
			if(tablaBuscada->entradaTablaPaginas[paginaApuntada].fueModificado == 0)
				return 1;
		if(*punteroClock < tablaBuscada->cantidadEntradasMemoria)
			*punteroClock=*punteroClock +1;
		else
			*punteroClock=0;
	}
	return 0 ;
}

//Busca Bit presencia = 0 && bit Modificado = 1
int buscaNoPresenciaSiModificado(t_tablaDePaginas *tablaBuscada,unsigned *punteroClock,unsigned pidActivo,unsigned indice)
{
	unsigned paginaApuntada;
	unsigned pagina;

	for(pagina = 0 ; pagina  < tablaBuscada->cantidadEntradasMemoria;pagina++ )
	{
		paginaApuntada = tablaBuscada->paginasEnMemoria[*punteroClock];

		//Busca si hay alguna pagina libre
		if(tablaBuscada->entradaTablaPaginas[paginaApuntada].estaEnMemoria == 0)
			if(tablaBuscada->entradaTablaPaginas[paginaApuntada].fueModificado == 1)
				return 1;
		//Si esta en memoria la libera
		if(tablaBuscada->entradaTablaPaginas[paginaApuntada].estaEnMemoria == 1)
			falloPagina(tablaBuscada,indice,pidActivo,paginaApuntada);

		//avanza el puntero
		if(*punteroClock < tablaBuscada->cantidadEntradasMemoria)
			*punteroClock=*punteroClock +1;
		else
			*punteroClock=0;
	}
	return 0;
}
t_tablaDePaginas* buscarTablaSegun(unsigned pidActivo,unsigned *indice)
{
	t_tablaDePaginas *tablaBuscada;
	for(*indice = 0; *indice < list_size(tablasDePaginas); *indice = *indice+1)
	{
		tablaBuscada = list_get(tablasDePaginas,*indice);
		if(tablaBuscada->pid==pidActivo)
		{
			return tablaBuscada;
		}
	}
	return tablaBuscada;

}
unsigned algoritmoClockMejorado(unsigned pidActivo,unsigned *indice)
{
	log_trace(logger,"Se ejecuta el algortimo clock mejorado");
	unsigned punteroClock;
	t_tablaDePaginas* tablaBuscada = buscarTablaSegun(pidActivo,indice);
	punteroClock = tablaBuscada->punteroClock;

	while(1)
	{
		if(buscaNoPresenciaNoModificado(tablaBuscada,&punteroClock,pidActivo)  == 1)
			return punteroClock;
		if(buscaNoPresenciaSiModificado(tablaBuscada,&punteroClock,pidActivo,*indice) == 1)
			return punteroClock;
	}
	return punteroClock;
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
int paginaEnEntrada(unsigned pagina,t_tablaDePaginas* tablaBuscada)
{
	unsigned posicionPagina;
	for(posicionPagina = 0 ; posicionPagina < tablaBuscada->cantidadEntradasMemoria; posicionPagina++)
	{
		if(tablaBuscada->paginasEnMemoria[posicionPagina] == pagina)
			return posicionPagina;
	}
	return -1;
}
unsigned algoritmoclock(unsigned pidActivo,unsigned *indice,unsigned pagina,int *paginaSiYaEstabaEnMemoria)
{
	log_trace(logger,"===========INICIO=CLOCK===================\n");
	log_trace(logger,"Se ejecuta el algoritmo clock\n");
	unsigned punteroClock;
	t_tablaDePaginas* tablaBuscada = buscarTablaSegun(pidActivo,indice);
	punteroClock = tablaBuscada->punteroClock;
	log_trace(logger,"EL puntero clock:%d\n",punteroClock);
	unsigned paginaApuntada;
	*paginaSiYaEstabaEnMemoria = paginaEnEntrada(pagina,tablaBuscada);

	//La pagina a reemplazar ya estaba en la tabla de las paginas en memoria
	if(*paginaSiYaEstabaEnMemoria != -1)
	{
		log_trace(logger,"==============FIN=CLOCK==================\n");
		return punteroClock;
	}
	while(1)
	{

		paginaApuntada = tablaBuscada->paginasEnMemoria[punteroClock];
		if(paginaApuntada == -1)
		{
			log_trace(logger,"==============FIN=CLOCK==================\n");
			return punteroClock;
		}
		if(tablaBuscada->entradaTablaPaginas[paginaApuntada].estaEnMemoria == 1)
		{
			falloPagina(tablaBuscada,*indice,pidActivo,paginaApuntada);

			if(punteroClock == tablaBuscada->cantidadEntradasMemoria -1 )
			{
				punteroClock=0;
				log_trace(logger,"Puntero clock :%d\n",punteroClock);
			}
			else
			{
				punteroClock++;
				log_trace(logger,"Puntero clock %d\n:",punteroClock);
			}
		}
		else
		{
			log_trace(logger,"==============FIN=CLOCK==================\n");
			return punteroClock;
		}
	}
	log_trace(logger,"==============FIN=CLOCK==================\n");
	return punteroClock;
}
int buscarMarcoDisponible()
{
	t_tablaDePaginas* tablaDePaginas;
	unsigned marcoDisponible =0;
	unsigned marcosUsados = 0;
	unsigned indice;
	unsigned pagina;
	for(indice=0; indice < list_size(tablasDePaginas); indice++)
	{
		tablaDePaginas = list_get(tablasDePaginas,indice);
		for(pagina = 0 ; pagina < tablaDePaginas->cantidadEntradasTablaPagina;pagina++)
		{
			if(tablaDePaginas->entradaTablaPaginas[pagina].estaEnMemoria == 1)
			{
				marcosUsados++;
				if(tablaDePaginas->entradaTablaPaginas[pagina].marco == marcoDisponible)
					marcoDisponible++;
			}
		}
	}
	if(marcosUsados == infoMemoria.marcos)
		return -1;
	else
		return marcoDisponible;
}


unsigned actualizaPagina(unsigned pagina,unsigned pidActivo,unsigned punteroClock,unsigned indice,int clienteUMC,int paginaEstabaEnMemoria)
{
	log_trace(logger,"Se actualiza la tabla de pagina del pid:%d",pidActivo);
	t_tablaDePaginas *tablaDePagina;
	t_tablaDePaginas *tablaPrueba;
	int marcoDisponible = buscarMarcoDisponible();
	if(marcoDisponible == -1)
	{
		log_error(logger,"No hay marcos disponibles, se notifica al CPU");
		enviarCodigoAlCPU(NULL,0,clienteUMC,3);
		pthread_mutex_unlock(&mutexTablaPaginas);
		pthread_exit(NULL);
		perror("No salio del hilo");
	}
	tablaDePagina = list_get(tablasDePaginas,indice);
	if(tablaDePagina->pid == pidActivo)
	{
		if(paginaEstabaEnMemoria != -1)
		{
			//Se actualiza la nueva pagina a las entradas en memoria
			tablaDePagina->paginasEnMemoria[punteroClock] = pagina;

			//Se actualiza el Puntero del clock
			if(punteroClock == tablaDePagina->cantidadEntradasMemoria -1)
				tablaDePagina->punteroClock = 0;
			else
				tablaDePagina->punteroClock = tablaDePagina->punteroClock+1;

			log_trace(logger,"El puntero clock luego del algortimo:%d",tablaDePagina->punteroClock);
		}
		else
			log_trace(logger,"La pagina ya se encuentra en las entradasDeMemoria");

		//Actualizar la tabla de paginas
		tablaDePagina->entradaTablaPaginas[pagina].estaEnMemoria = 1;
		tablaDePagina->entradaTablaPaginas[pagina].marco = marcoDisponible;
		list_replace(tablasDePaginas,indice,tablaDePagina);
		tablaPrueba = list_get(tablasDePaginas,indice);
		if(tablaPrueba->entradaTablaPaginas[pagina].estaEnMemoria == 1 && tablaPrueba->punteroClock == tablaDePagina->punteroClock)
			log_trace(logger,"La tabla de pagina se actualizo correctamente");
		else
			log_error(logger,"La tabla de paginas No se actualizo correctamente");

		return tablaDePagina->entradaTablaPaginas[pagina].marco;
	}
	return tablaDePagina->entradaTablaPaginas[pagina].marco;
}

void escribirEnMemoria(void* codigoPrograma,unsigned tamanioPrograma, unsigned pagina,unsigned pidActivo,unsigned punteroClock,unsigned indice,int clienteUMC,int paginaEstabaEnMemoria)
{
	unsigned marco = actualizaPagina(pagina,pidActivo,punteroClock,indice,clienteUMC,paginaEstabaEnMemoria);
	pthread_mutex_lock(&mutexMemoria);
	log_trace(logger,"Se actualiza la informacion del marco:%d",marco);
	memcpy(memoriaPrincipal + infoMemoria.tamanioDeMarcos*marco,codigoPrograma,tamanioPrograma);
	if(strncmp(codigoPrograma,memoriaPrincipal + infoMemoria.tamanioDeMarcos*marco,tamanioPrograma) == 0)
		log_trace(logger,"Se actualizo correctamente la informacion del marco");
	else
		log_error(logger,"Hubo un problema en actualizar la informacion del marco");

	log_trace(logger,"contenido actual de la memoriaṔrincial:\n%s",memoriaPrincipal);
	pthread_mutex_unlock(&mutexMemoria);
	free(codigoPrograma);
	return;
}

void algoritmoDeReemplazo(void* codigoPrograma,unsigned tamanioPrograma,unsigned pagina,unsigned pidActivo,int clienteUMC)
{
	pthread_mutex_lock(&mutexClock);
	unsigned punteroClock;
	int paginaEstabaEnMemoria;
	unsigned indice;

	//Eleccion entre Algoritmos
	if(!strcmp("CLOCK",infoConfig.algoritmo))
		punteroClock = algoritmoclock(pidActivo,&indice,pagina,&paginaEstabaEnMemoria);

	if(!strcmp("CLOCKMEJORADO",infoConfig.algoritmo))
		punteroClock = algoritmoClockMejorado(pidActivo,&indice);


	//Escribe en memoria la nueva pagina que mando el SWAP
	escribirEnMemoria(codigoPrograma,tamanioPrograma,pagina,pidActivo,punteroClock,indice,clienteUMC,paginaEstabaEnMemoria);
	pthread_mutex_unlock(&mutexClock);

	return ;
}

void pedirPagAlSWAP(unsigned pagina,unsigned pidActual) {
	log_trace(logger,"pedirPagAlSWAP()");
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
	log_trace(logger,"fin -> pedirPagAlSWAP()");
}

void traerPaginaAMemoria(unsigned pagina,unsigned pidActual,int clienteUMC)
{
	t_mensaje aRecibir;

	//Pedimos pagina al SWAP
	log_trace(logger,"pedimos la pagina:%d con pid:%d al SWAP",pagina,pidActual);
	pedirPagAlSWAP(pagina,pidActual);

	//Recibimos pagina del SWAP
	recibirMensaje(clienteSWAP, &aRecibir);
	log_trace(logger,"Codigo recibido: %u", aRecibir.head.codigo);
	log_trace(logger,"Mensaje_extra  :\n %s",aRecibir.mensaje_extra);
	log_trace(logger,"Mensaje_extra_tam:%d",aRecibir.head.tam_extra);
	algoritmoDeReemplazo(aRecibir.mensaje_extra,aRecibir.head.tam_extra,pagina,pidActual,clienteUMC);
	log_trace(logger,"pase algoritmoDeReemplazo");

	return;
}

void actualizarTLB(t_entradaTablaPaginas entradaDePaginas,unsigned pagina,unsigned pidActual)
{
	pthread_mutex_lock(&mutexTLB);
	log_trace(logger,"actualizarTLB();");
	//LRU
	int tamanioMaximoTLB = infoMemoria.entradasTLB;
	int tamanioTLB = list_size(TLB);

	t_entradaTLB *entradaTLB = malloc(sizeof(t_entradaTLB));
	entradaTLB->pid = pidActual;
	entradaTLB->pagina = pagina;
	entradaTLB->estaEnMemoria = entradaDePaginas.estaEnMemoria;
	entradaTLB->fueModificado = entradaDePaginas.fueModificado;
	entradaTLB->marco         = entradaDePaginas.marco;

	if(tamanioTLB == tamanioMaximoTLB)
		list_replace(TLB,tamanioTLB-1,entradaTLB);
	else
		list_add_in_index(TLB,tamanioTLB,entradaTLB);

	log_trace(logger,"Fin ActualizarTLB();");
	pthread_mutex_unlock(&mutexTLB);
	return;
}

int buscarEnTLB(unsigned paginaBuscada,unsigned pidActual)
{
	int indice;
	int marco;
	t_entradaTLB *entradaTLB;
	for(indice=0;indice < list_size(TLB);indice++)
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

void traducirPaginaAMarco(unsigned pagina,int *marco,unsigned pidActual,int clienteUMC)
{

	unsigned indice;
	pthread_mutex_lock(&mutexTablaPaginas);  //cuidado con los mutex boludo!
	t_tablaDePaginas *tablaDePaginas;

	log_trace(logger,"Busca si la pagina esta en la TLB");
	//Buscar en TLB
	*marco = buscarEnTLB(pagina,pidActual);
	if(*marco != -1)
	{
		log_trace(logger,"Pagina en TLB, marco correspondiente: %d",*marco);
		pthread_mutex_unlock(&mutexTablaPaginas);
		return;
	}

	log_trace(logger,"Busca la pagina en la tabla de paginas");
	//Buscar en tabla de paginas
	tablaDePaginas = buscarTablaSegun(pidActual,&indice);
	if(tablaDePaginas->entradaTablaPaginas[pagina].estaEnMemoria == 1)
		{
			//Esta en memoria se copia el marco
			log_trace(logger,"La pagina se encuentra en memoria");
			*marco = tablaDePaginas->entradaTablaPaginas[pagina].marco;
			log_trace(logger,"El marco correspondiente: %d", *marco);
			pthread_mutex_unlock(&mutexTablaPaginas);
			log_trace(logger,"Se actualiza la TLB");
			actualizarTLB(tablaDePaginas->entradaTablaPaginas[pagina],pagina,pidActual);
			return;
		}
		else
		{	// Buscar en swap
			log_trace(logger,"La pagina no se encuentra en memoria, se procede a traer del SWAP");
			traerPaginaAMemoria(pagina,pidActual,clienteUMC);
			*marco = tablaDePaginas->entradaTablaPaginas[pagina].marco;
			log_trace(logger,"El marco correspondiente: %d", *marco);
			pthread_mutex_unlock(&mutexTablaPaginas);
			return;
		}



	return;
}

void almacenarBytes(int *marco,unsigned paginasALeer,unsigned offset,unsigned tamanio,void* codigo)
{
	unsigned aRecorrer;
	unsigned seLeyo;
	//Se guarda el contenido del primer marco
	memcpy(memoriaPrincipal+infoMemoria.tamanioDeMarcos*marco[0]+offset,codigo,infoMemoria.tamanioDeMarcos-offset);

	if(paginasALeer == 1)
		return;

	//Se copia el contenido de los marcos intermedios
	tamanio = tamanio - (infoMemoria.tamanioDeMarcos - offset);
	seLeyo = infoMemoria.tamanioDeMarcos - offset;
	for(aRecorrer = 1;aRecorrer +1 < paginasALeer ;aRecorrer++)
	{
		memcpy(memoriaPrincipal+infoMemoria.tamanioDeMarcos*marco[aRecorrer],codigo + seLeyo,infoMemoria.tamanioDeMarcos);
		tamanio = tamanio - infoMemoria.tamanioDeMarcos;
		seLeyo = seLeyo + infoMemoria.tamanioDeMarcos;
	}

	//Se copia el contenido del ultimo marco
	memcpy(memoriaPrincipal+infoMemoria.tamanioDeMarcos*marco[paginasALeer-1],codigo+seLeyo,tamanio);


	return;
}


void procesosEnTabla()
{
	unsigned proceso;
	pthread_mutex_lock(&mutexTablaPaginas);
	t_tablaDePaginas *tabla;
	unsigned pagina;
	if(list_size(tablasDePaginas) == 0){
		pthread_mutex_unlock(&mutexTablaPaginas);
		return;
	}
	log_trace(logger,"============================\n");
	log_trace(logger,"Procesos en Tablas \n");
	for(proceso= 0; proceso < list_size(tablasDePaginas);proceso++)
	{
		tabla = list_get(tablasDePaginas,proceso);
		log_trace(logger,"--------------------------------\n");
		log_trace(logger,"Proceso pid: %d \n paginas:%d \n estan en memoria:%d \n punteroClock:%d \n",tabla->pid,tabla->cantidadEntradasTablaPagina,tabla->punteroClock);
		log_trace(logger,"Paginas:\n");

		for(pagina = 0;pagina < tabla->cantidadEntradasTablaPagina;pagina++)
		{
			if(tabla->entradaTablaPaginas[pagina].estaEnMemoria == 1)
				log_trace(logger,"-Pagina:%d -> Marco:%d\n",pagina,tabla->entradaTablaPaginas[pagina].marco);
			else
				log_trace(logger,"-Pagina:%d -> Marco:NULL\n",pagina);
		}
		log_trace(logger,"--------------------------------\n");
	}
	log_trace(logger,"============================\n");
	pthread_mutex_unlock(&mutexTablaPaginas);
	return;
}
void enviarCodigoAlCPU(char* codigoAEnviar, unsigned tamanio,int clienteUMC,unsigned estado)
{
	t_mensaje mensaje;
	unsigned parametros[1];
	parametros[0] = estado;
	mensaje.head.codigo = RETURN_OK;
	mensaje.head.cantidad_parametros = 1;
	mensaje.head.tam_extra = tamanio;
	mensaje.parametros = parametros;
	mensaje.mensaje_extra = codigoAEnviar;
	empaquetarYEnviar(mensaje,clienteUMC);

	return;
}

unsigned paginasARecorrer(unsigned offset, unsigned tamanio)
{
	unsigned tamanioTotal = offset+tamanio;
	if(tamanioTotal % infoMemoria.tamanioDeMarcos == 0)
	   return tamanioTotal / infoMemoria.tamanioDeMarcos;
	else
	   return tamanioTotal / infoMemoria.tamanioDeMarcos + 1;
}

unsigned copiarCodigo(unsigned paginaDondeEmpieza,unsigned paginasALeer,unsigned pidActual,int clienteUMC,unsigned tamanio,unsigned offset,void* codigoAEnviar)
{
	unsigned paginaATraducir;
	int marco;
	unsigned tamanioACopiar = infoMemoria.tamanioDeMarcos - offset;
	unsigned seLeyo = 0;

	t_tablaDePaginas *tablaDePaginas = buscarTablaSegun(pidActual,&paginaATraducir);

	if(paginaDondeEmpieza + paginasALeer <= tablaDePaginas->cantidadEntradasTablaPagina)
	{
		log_trace(logger,"Las %d paginas a leer estan incluidas en el proceso",paginasALeer);

		for(paginaATraducir = 0; paginaATraducir < paginasALeer;paginaATraducir++)
		{

			log_trace(logger,"Se traduce la pagina: %d con pid:%d al marco correspondiente \n" ,paginaDondeEmpieza+paginaATraducir,pidActual);
			traducirPaginaAMarco(paginaDondeEmpieza+paginaATraducir,&marco,pidActual,clienteUMC);

			log_trace(logger,"Se copia el contenido de la pagina:%d pid:%d marco:%d",paginaDondeEmpieza+paginaATraducir,pidActual,marco);
			memcpy(codigoAEnviar+seLeyo,memoriaPrincipal+infoMemoria.tamanioDeMarcos*marco+offset,tamanioACopiar);
			log_trace(logger,"Se copio:\n %s",codigoAEnviar);
			offset = 0;
			seLeyo = seLeyo + tamanioACopiar;
			tamanio = tamanio - tamanioACopiar;
			//Se fija si esta por leer la ultima pagina
			if(paginaATraducir < paginasALeer -1)
				tamanioACopiar = infoMemoria.tamanioDeMarcos;
			else
				tamanioACopiar = tamanio;
		}
		return 1;
	}
	else
	{
		log_error(logger,"Paginas fuera de segmento, se notifica al CPU");
		return 2;
	}
}


unsigned guardarCodigo(unsigned paginaDondeEmpieza,unsigned paginasALeer,unsigned pidActual,int clienteUMC,unsigned tamanio,unsigned offset,void* codigoAGuardar)
{
	unsigned paginaATraducir;
	int marco;
	unsigned tamanioACopiar = infoMemoria.tamanioDeMarcos - offset;
	unsigned seLeyo = 0;

	t_tablaDePaginas *tablaDePaginas = buscarTablaSegun(pidActual,&paginaATraducir);

	if(paginaDondeEmpieza + paginasALeer <= tablaDePaginas->cantidadEntradasTablaPagina)
	{
		log_trace(logger,"Las %d paginas a leer estan incluidas en el proceso",paginasALeer);

		for(paginaATraducir = 0; paginaATraducir < paginasALeer;paginaATraducir++)
		{

			log_trace(logger,"Se traduce la pagina: %d con pid:%d al marco correspondiente \n" ,paginaDondeEmpieza+paginaATraducir,pidActual);
			traducirPaginaAMarco(paginaDondeEmpieza+paginaATraducir,&marco,pidActual,clienteUMC);

			log_trace(logger,"Se copia el contenido de la pagina:%d pid:%d marco:%d",paginaDondeEmpieza+paginaATraducir,pidActual,marco);
			memcpy(memoriaPrincipal+infoMemoria.tamanioDeMarcos*marco+offset,codigoAGuardar+seLeyo,tamanioACopiar);
			offset = 0;
			seLeyo = seLeyo + tamanioACopiar;
			tamanio = tamanio - tamanioACopiar;

			//Se fija si esta por leer la ultima pagina
			if(paginaATraducir < paginasALeer -1)
				tamanioACopiar = infoMemoria.tamanioDeMarcos;
			else
				tamanioACopiar = tamanio;
		}
		return 1;
	}
	else
	{
		log_error(logger,"Paginas fuera de segmento, se notifica al CPU");
		return 2;
	}
}

void almacenarBytesEnPagina(t_mensaje mensaje,unsigned pidActivo, int clienteUMC)
{
	unsigned pagina  = mensaje.parametros[0];
	unsigned offset  = mensaje.parametros[1];
	unsigned tamanio = mensaje.parametros[2];
	void* codigo = malloc(tamanio);
	log_trace(logger,"Se procede a almacenar -> pagina:%d, offset:%d, tamaño:%d, pid:%d",pagina,offset,tamanio,pidActivo);
	memcpy(codigo,&mensaje.parametros[3],tamanio);

	unsigned paginasALeer = paginasARecorrer(offset,tamanio);
	unsigned estado = guardarCodigo(pagina,paginasALeer,pidActivo,clienteUMC,tamanio,offset,codigo);

	enviarCodigoAlCPU(codigo,tamanio,clienteUMC,estado);

	free(codigo);
	return;
}

void enviarBytesDeUnaPagina(t_mensaje mensaje,int clienteUMC,unsigned pidActual)
{
	log_trace(logger,"Peticion de envio de codigo al cpu");
	unsigned pagina  = mensaje.parametros[0];
	unsigned offset  = mensaje.parametros[1];
	unsigned tamanio = mensaje.parametros[2];
	log_trace(logger,"\n Pagina:%d \n Offset:%d \n Tamaño:%d",pagina,offset,tamanio);
	unsigned paginasALeer = paginasARecorrer(offset,tamanio);
	void* codigoAEnviar = malloc(tamanio);
	unsigned estado = copiarCodigo(pagina,paginasALeer,pidActual,clienteUMC,tamanio,offset,codigoAEnviar);


	log_trace(logger,"Se envia la instruccion al CPU");
	log_trace(logger,"Instruccion:\n   %s",codigoAEnviar);
	enviarCodigoAlCPU(codigoAEnviar,tamanio,clienteUMC,estado);
	log_trace(logger,"Fin Enviar instruccion al CPU");

	free(codigoAEnviar);

	return;
}


void enviarTamanioDePagina(int clienteUMC)
{
	t_mensaje mensaje;
	unsigned parametros[1];
	parametros[0] = infoMemoria.tamanioDeMarcos;
	mensaje.head.codigo = RETURN_TAM_PAGINA;
	mensaje.head.cantidad_parametros = 1;
	mensaje.head.tam_extra = 0;
	mensaje.parametros = parametros;
	mensaje.mensaje_extra = NULL;
	empaquetarYEnviar(mensaje,clienteUMC);

	return;
}

void accionSegunCabecera(int clienteUMC,unsigned pid)
{
	log_trace(logger,"Se creo un Hilo");
	unsigned pidActivo = pid;
	int cabeceraDelMensaje;
	t_mensaje mensaje;

	while(1){
		procesosEnTabla();
		if(recibirMensaje(clienteUMC,&mensaje) <= 0){
			clienteDesconectado(clienteUMC);
		}
		cabeceraDelMensaje = mensaje.head.codigo;
		log_trace(logger,"Codigo Recibido: %u", cabeceraDelMensaje);
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
				pidActivo = cambioProcesoActivo(mensaje.parametros[0],pidActivo);
				break;
			case RECORD_DATA:
				almacenarBytesEnPagina(mensaje,pidActivo, clienteUMC);
				break;
		}
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
			if( FD_ISSET(fdBuscador,&fdsParaLectura) )  //entra una conexion, o hay datos entrantes
			{
				if(fdBuscador == servidorUMC)
				{
					clienteUMC = aceptarConexion();
					FD_SET(clienteUMC, &master);
					if(clienteUMC > maximoFD) //Actualzar el maximo fd
						maximoFD = clienteUMC;
					log_trace(logger,"ID Hilo: %i", clienteUMC);
					log_trace(logger,"Se crea un hilo");
					pthread_create(&cliente, NULL, (void *) gestionarSolicitudesDeOperacion, (void *) clienteUMC);
				} 
				else //Se recibieron datos de otro tipo
				{
					//Hacer lo que haga falta
				}

			} else {
				

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
