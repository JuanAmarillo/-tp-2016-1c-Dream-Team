#include "umc.h"



void leerArchivoConfig()
{
	system("clear");
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
void destruirTLB(t_entradaTLB* entradaTLB)
{
	free(entradaTLB);
}
void destruirTablaPaginas(t_tablaDePaginas* entradaTablaPagina)
{
	free(entradaTablaPagina->entradaTablaPaginas);
	free(entradaTablaPagina->paginasEnMemoria);
	free(entradaTablaPagina);
}
void finalizarUMC()
{
	pthread_mutex_lock(&mutexClientes);
	pthread_mutex_lock(&mutexClock);
	pthread_mutex_lock(&mutexMemoria);
	pthread_mutex_lock(&mutexTLB);
	pthread_mutex_lock(&mutexTablaPaginas);

	log_trace(logger,"Nucleo desconectado se finaliza la UMC ");
	free(memoriaPrincipal);
	list_destroy_and_destroy_elements(TLB,(void*)destruirTLB);
	list_destroy_and_destroy_elements(tablasDePaginas,(void*)destruirTablaPaginas);
	abort();

	return;
}
void inicializarMarcos()
{
	unsigned marco;
	for(marco=0;marco < infoMemoria.marcos;marco++)
	{
		marcoDisponible[marco] = 0;
		marcoAsignadoA[marco]  = 0;
	}
	return;
}
void inicializarEstructuras()
{
	logger = log_create("UMC_TEST.txt", "UMC", 0, LOG_LEVEL_TRACE);
	loggerVariables = log_create("UMC_VAR.txt","UMC",0,LOG_LEVEL_TRACE);
	logger1 = log_create("UMC_CONSOLA.txt","UMC",1,LOG_LEVEL_TRACE);
	loggerTLB  = log_create("UMC_TLB.txt","UMC",0,LOG_LEVEL_TRACE);
	loggerClock = log_create("UMC_CLOCK.txt","UMC",0,LOG_LEVEL_TRACE);
	clienteNucleo = 10;
	memoriaPrincipal = malloc(infoMemoria.marcos * infoMemoria.tamanioDeMarcos);
	TLB = list_create();
	tablasDePaginas  = list_create();

	marcoAsignadoA =  malloc(infoMemoria.marcos*sizeof(int));
	marcoDisponible = malloc(infoMemoria.marcos*sizeof(int));
	inicializarMarcos();
	paginaVariablesTest = 99999;
	return;
}

void clienteDesconectado(int clienteUMC)
{
	pthread_mutex_lock(&mutexClientes);
	FD_CLR(clienteUMC, &master);
	pthread_mutex_unlock(&mutexClientes);

	if(clienteUMC == clienteNucleo){
		close(clienteUMC);
		finalizarUMC();
	}
	log_trace(logger,"CPU desconectado,Se borra el hilo");
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
	t_mensaje codigo;
	unsigned parametrosParaEnviar[1];

	//Enviar programa al SWAP
	codigo.head.codigo = SAVE_PROGRAM;
	codigo.head.cantidad_parametros = 1;
	parametrosParaEnviar[0] = pid;
	codigo.parametros = parametrosParaEnviar;
	codigo.head.tam_extra = paginasSolicitadas * infoMemoria.tamanioDeMarcos;

	codigo.mensaje_extra = codigoPrograma;

	log_trace(logger,"%s", codigo.mensaje_extra);
	enviarMensaje(clienteSWAP, codigo);
	//free(codigo.mensaje_extra);
}

void enviarSuficienteEspacio(int clienteUMC, int codigo)
{
	t_mensaje noEspacio;
	unsigned parametros[1];
	parametros[0] = 1;
	noEspacio.head.codigo = codigo;
	noEspacio.head.cantidad_parametros = 1;
	noEspacio.head.tam_extra = 0;
	noEspacio.parametros = parametros;
	empaquetarYEnviar(noEspacio,clienteUMC);
}

unsigned enviarCodigoAlSwap(unsigned paginasSolicitadas,char* codigoPrograma,unsigned pid,unsigned tamanioCodigo,int clienteUMC)
{
	t_mensaje respuesta,respuestaSaveProgram;

	pthread_mutex_lock(&mutexClientes);

	//Reservar espacio en el SWAP
	log_trace(logger,"Pide espacio para reservar el programa al SWAP");
	pedirReservaDeEspacio(pid, paginasSolicitadas);

	//fijarse si pudo reservar
	recibirMensaje(clienteSWAP,&respuesta);

	if(respuesta.head.codigo == NOT_ENOUGH_SPACE)
	{
		log_trace(logger,"No hay suficiente espacio para almacenar el programa");
		pthread_mutex_unlock(&mutexClientes);
		enviarSuficienteEspacio(clienteUMC,ALMACENAR_FAILED);

		free(codigoPrograma);
		freeMensaje(&respuesta);
		return 0;
	}

	//Enviar programa al SWAP
	log_trace(logger,"Hay suficiente espacio, se envia el programa al SWAP");
	enviarProgramaAlSWAP(pid, paginasSolicitadas, tamanioCodigo, codigoPrograma);
	recibirMensaje(clienteSWAP,&respuestaSaveProgram);
	pthread_mutex_unlock(&mutexClientes);

	free(codigoPrograma);
	freeMensaje(&respuesta);
	freeMensaje(&respuestaSaveProgram);

	return 1;
}

void crearTablaDePaginas(unsigned pid,unsigned paginasSolicitadas)
{
	log_trace(logger,"Se crea una tabla de paginas PID:%d, Paginas:%d",pid,paginasSolicitadas);
	int pagina;

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
			tablaPaginas->paginasEnMemoria[pagina] = -1;

	}
	for(pagina=0;pagina < paginasSolicitadas; pagina++)
	{
		tablaPaginas->entradaTablaPaginas[pagina].estaEnMemoria = 0;
		tablaPaginas->entradaTablaPaginas[pagina].fueModificado = 0;
	}

	usleep(infoMemoria.retardo);
	pthread_mutex_lock(&mutexTablaPaginas);
	list_add(tablasDePaginas,tablaPaginas);
	pthread_mutex_unlock(&mutexTablaPaginas);
	return;

}
void mostrarTLB()
{
	pthread_mutex_lock(&mutexTLB);
	unsigned index ;
	t_entradaTLB *entradaTLB;
	log_trace(loggerTLB,"=======================");
	log_trace(loggerTLB,"Entradas en TLB:");
	for(index = 0; index < list_size(TLB) ; index++)
	{
		entradaTLB = list_get(TLB,index);
		log_trace(loggerTLB,"pid:%d \n pagina:%d  \n marco:%d",entradaTLB->pid,entradaTLB->marco);
	}
	log_trace(loggerTLB,"=======================");
	pthread_mutex_unlock(&mutexTLB);
	return;
}

void borrarEntradasTLBSegun(unsigned pidActivo)
{
	pthread_mutex_lock(&mutexTLB);
	unsigned entrada = 0;
	t_entradaTLB *entradaTLB;
	log_trace(loggerTLB,"Se borran las entradas del pid:%d",pidActivo);
	while(list_size(TLB) != 0)
	{
		if(entrada == list_size(TLB))
		{
			pthread_mutex_unlock(&mutexTLB);
			mostrarTLB();
			return;
		}

		entradaTLB = list_get(TLB,entrada);
		if(entradaTLB->pid == pidActivo)
		{
			log_trace("Se borra la entradaTLB PID:%d Pagina:%d Marco:%d",entradaTLB->pid,entradaTLB->pagina,entradaTLB->marco);
			list_remove(TLB,entrada);
		}
		else
			entrada++;
	}
	pthread_mutex_unlock(&mutexTLB);
	mostrarTLB();
	return;
}
t_tablaDePaginas* buscarTabla(unsigned pidActivo,unsigned *indice,unsigned *seEncontro)
{
	pthread_mutex_lock(&mutexTablaPaginas);
	t_tablaDePaginas *tablaBuscada = NULL;
	for(*indice = 0; *indice < list_size(tablasDePaginas); *indice = *indice+1)
	{
		tablaBuscada = list_get(tablasDePaginas,*indice);
		if(tablaBuscada->pid==pidActivo)
		{
			pthread_mutex_unlock(&mutexTablaPaginas);
			*seEncontro = 1;
			return tablaBuscada;
		}
	}
	pthread_mutex_unlock(&mutexTablaPaginas);
	*seEncontro = 0;
	return tablaBuscada;
}
t_tablaDePaginas* buscarTablaSegun(unsigned pidActivo,unsigned *indice)
{
	t_tablaDePaginas*tablaBuscada = NULL;
	unsigned seEncontro;
	tablaBuscada = buscarTabla(pidActivo,indice,&seEncontro);
	if(seEncontro == 0)
	{
		pthread_exit(NULL);
	}
	return tablaBuscada;

}

t_tablaDePaginas* cambioProcesoActivo(unsigned pid,unsigned pidActivo)
{
	log_trace(logger,"======Se cambia de proceso======");
	log_trace(logger,"PID:%d por PID:%d",pid,pidActivo);
	unsigned indice;
	usleep(infoMemoria.retardo);
	t_tablaDePaginas* procesoActivo = buscarTablaSegun(pid,&indice);
	if(pidActivo > 0)
	{
		log_trace(logger,"Se borran las entradas de la TLB del pid anterior");
		borrarEntradasTLBSegun(pidActivo);
	}
	log_trace(logger,"=================================================");
	return procesoActivo ;
}

void inicializarPrograma(t_mensaje mensaje,int clienteUMC)
{
	unsigned pid = mensaje.parametros[0];
	unsigned paginasSolicitadas = mensaje.parametros[1];
	char* codigoPrograma = malloc(paginasSolicitadas*infoMemoria.tamanioDeMarcos);
	memcpy(codigoPrograma, mensaje.mensaje_extra, mensaje.head.tam_extra);
	unsigned tamanioCodigo=mensaje.head.tam_extra ;

	log_trace(logger,"======Se Inicializa un nuevo Proceso======",pid,paginasSolicitadas);
	log_trace(logger,"-PID: %d",pid);
	log_trace(logger,"-Paginas: %d",paginasSolicitadas);

	if(enviarCodigoAlSwap(paginasSolicitadas,codigoPrograma,pid,tamanioCodigo,clienteUMC) == 1){
		crearTablaDePaginas(pid,paginasSolicitadas);
		enviarSuficienteEspacio(clienteUMC,ALMACENAR_OK);
	}
	log_trace(logger,"===========================================");
	return;

}
void liberarMarcos(t_tablaDePaginas *proceso)
{
	int pagina;
	int paginaAPuntada;
	for(pagina = 0; pagina < proceso->cantidadEntradasMemoria;pagina++)
	{
		paginaAPuntada = proceso->paginasEnMemoria[pagina];
		if(paginaAPuntada != -1)
		{
			log_trace(logger,"Se libera el marco:%d",proceso->entradaTablaPaginas[paginaAPuntada].marco);
			marcoDisponible[proceso->entradaTablaPaginas[paginaAPuntada].marco] = 0;
			marcoAsignadoA[proceso->entradaTablaPaginas[paginaAPuntada].marco] = 0;
		}
	}
	return;
}
void eliminarDeMemoria(unsigned pid,unsigned* huboFallo)
{
	t_tablaDePaginas *buscador;
	unsigned index;
	unsigned seEncontro;
	usleep(infoMemoria.retardo);
	buscador = buscarTabla(pid,&index,&seEncontro);
	if(seEncontro == 1)
	{
		pthread_mutex_lock(&mutexTablaPaginas);
		liberarMarcos(buscador);
		log_trace(logger,"Se destruye la tabla de paginas PID:%d",pid);
		list_remove_and_destroy_element(tablasDePaginas,index,(void*)destruirTablaPaginas);
		pthread_mutex_unlock(&mutexTablaPaginas);
	}
	else
	{
		log_error(logger,"El nucleo envio un PID ya eliminado");
		*huboFallo = 1;
	}
	return;
}

void finPrograma(t_mensaje finalizarProg)
{
	log_trace(logger,"======Se finaliza un Programa======");
	unsigned huboFallo = 0;
	unsigned pid = finalizarProg.parametros[0];
	log_trace(logger,"-PID:%d",pid);
	eliminarDeMemoria(pid,&huboFallo);
	if(huboFallo == 0)
		enviarMensaje(clienteSWAP,finalizarProg);
	log_trace(logger,"===================================");
	return;
}
void borrarEntradaTLB(int marco)
{
	pthread_mutex_lock(&mutexTLB);
	unsigned entrada;
	t_entradaTLB* entradaTLB;
	for(entrada = 0 ; entrada < list_size(TLB); entrada++)
	{
		entradaTLB = list_get(TLB,entrada);
		if(entradaTLB->marco == marco)
		{
			log_trace(loggerTLB,"Se borra la entradaTLB que contiene el marco:%d",marco);
			list_remove_and_destroy_element(TLB,entrada,(void*)destruirTLB);
			pthread_mutex_unlock(&mutexTLB);
			return;
		}

	}
	pthread_mutex_unlock(&mutexTLB);
	return;
}

void falloPagina(t_tablaDePaginas* procesoActivo,unsigned paginaApuntada)
{
	log_trace(loggerClock,"Fallo de Pagina:  Pagina:%d PID:%d",paginaApuntada,procesoActivo->pid);
	void* codigoDelMarco = malloc(infoMemoria.tamanioDeMarcos);
	unsigned marco = procesoActivo->entradaTablaPaginas[paginaApuntada].marco ;

	//Copio el codigo del marco
	usleep(infoMemoria.retardo);
	pthread_mutex_lock(&mutexMemoria);
	memcpy(codigoDelMarco, memoriaPrincipal+infoMemoria.tamanioDeMarcos*marco , infoMemoria.tamanioDeMarcos);
	pthread_mutex_unlock(&mutexMemoria);

	//TEST
	if(paginaApuntada== paginaVariablesTest)
	{
		int offset;
		log_trace(loggerVariables,"Fallo Pagina: Pagina %d",paginaApuntada);
		for(offset = 0; offset < 9;offset=offset+4){
		void* codigoTest = malloc(4);
		memcpy(codigoTest,codigoDelMarco+offset,4);
		log_trace(loggerVariables,"Var %d: int:%d",offset/4,*((int*)codigoTest));
		}
	}
	//--
	//LLevo la pagina al SWAP
	log_trace(logger,"Se envia la Pagina:%d del Proceso:%d al SWAP",paginaApuntada,procesoActivo->pid);
	enviarPaginaAlSWAP(paginaApuntada,codigoDelMarco,procesoActivo->pid);

	free(codigoDelMarco);
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
unsigned avanzaPunteroClock(t_tablaDePaginas *tablaPaginas, unsigned punteroClock)
{
	if(punteroClock == tablaPaginas->cantidadEntradasMemoria -1 )
	{
		punteroClock=0;
		log_trace(loggerClock,"Avanza Puntero clock:%d PID:%d",punteroClock,tablaPaginas->pid);
	}
	else
	{
		punteroClock++;
		log_trace(loggerClock,"Avanza Puntero clock:%d PID:%d",punteroClock,tablaPaginas->pid);
	}

	return punteroClock;
}

int buscarMarcoDisponible()
{
	int marco;
	for(marco = 0 ; marco < infoMemoria.marcos; marco++)
	{
		if(marcoDisponible[marco] == 0 && marcoAsignadoA[marco] == 0)
		{
			return marco;
		}
	}
	return -1;
}

//Busca Bit presencia = 0 && bit Modificado = 0
int buscaNoPresenciaNoModificado(t_tablaDePaginas *procesoActivo,unsigned *punteroClock,unsigned pagina,int *paginaSiYaEstabaEnMemoria)
{
	unsigned paginaApuntada;
	*paginaSiYaEstabaEnMemoria = paginaEnEntrada(pagina,procesoActivo);
	int marco = buscarMarcoDisponible();


	//La pagina a reemplazar ya estaba en la tabla de las paginas en memoria
	if(*paginaSiYaEstabaEnMemoria != -1)
		return 1;


	for(pagina = 0 ; pagina  < procesoActivo->cantidadEntradasMemoria;pagina++ )
	{
		//No hay pagina cargada => no hay reemplazo
		paginaApuntada = procesoActivo->paginasEnMemoria[*punteroClock];
		if(paginaApuntada == -1)
		{
			if(marco != -1)
			{
				log_trace(loggerClock,"Encuentra un marco vacio");
				return 1;
			}
			else
			{
				*punteroClock = avanzaPunteroClock(procesoActivo,*punteroClock);
			}
		}
		else
		{
			//Busca una pagina que no fue modificada ni que este en memoria
			if(procesoActivo->entradaTablaPaginas[paginaApuntada].estaEnMemoria == 0)
				if(procesoActivo->entradaTablaPaginas[paginaApuntada].fueModificado == 0)
				{
					log_trace(loggerClock,"Encuentra una Pagina Bit Presencia = 0 & Bit Modificado = 0");
					return 1;
				}
			*punteroClock = avanzaPunteroClock(procesoActivo,*punteroClock);
		}
	}
	return 0 ;
}

//Busca Bit presencia = 0 && bit Modificado = 1
int buscaNoPresenciaSiModificado(t_tablaDePaginas *tablaBuscada,unsigned *punteroClock)
{
	unsigned paginaApuntada;
	unsigned pagina;
	int marco;

	for(pagina = 0 ; pagina  < tablaBuscada->cantidadEntradasMemoria;pagina++ )
	{
		marco = buscarMarcoDisponible();
		paginaApuntada = tablaBuscada->paginasEnMemoria[*punteroClock];

		//Busca si hay alguna pagina libre
		if(tablaBuscada->entradaTablaPaginas[paginaApuntada].estaEnMemoria == 0)
			if(tablaBuscada->entradaTablaPaginas[paginaApuntada].fueModificado == 1)
			{
				log_trace(loggerClock,"Encuentra una Pagina Bit Presencia = 0 & Bit Modificado = 1");
					return 1;
			}
		//Si esta en memoria la libera
		if(tablaBuscada->entradaTablaPaginas[paginaApuntada].estaEnMemoria == 1)
		{
			log_trace(loggerClock,"Bit Presencia Pagina:%d se pone en 0",paginaApuntada);
			tablaBuscada->entradaTablaPaginas[paginaApuntada].estaEnMemoria = 0;
			borrarEntradaTLB(tablaBuscada->entradaTablaPaginas[paginaApuntada].marco);
		}

		*punteroClock = avanzaPunteroClock(tablaBuscada,*punteroClock);
	}
	return 0;
}

unsigned algoritmoClockMejorado(t_tablaDePaginas*procesoActivo,unsigned pagina,int *paginaSiYaEstabaEnMemoria)
{
	log_trace(loggerClock,"===========INICIO=CLOCK-MEJORADO===================");
	unsigned punteroClock;
	punteroClock = procesoActivo->punteroClock;

	log_trace(loggerClock,"Puntero clock:%d PID:%d",punteroClock,procesoActivo->pid);

	while(1)
	{
		if(buscaNoPresenciaNoModificado(procesoActivo,&punteroClock,pagina,paginaSiYaEstabaEnMemoria)  == 1)
			return punteroClock;
		if(buscaNoPresenciaSiModificado(procesoActivo,&punteroClock) == 1)
			return punteroClock;
	}
	log_trace(loggerClock,"===========FIN=CLOCK-MEJORADO===================");
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
void abortarSiNoHayPaginasEnMemoria(t_tablaDePaginas* procesoActivo,int clienteUMC,int* noHayMarcos)
{
	unsigned pagina;
	for(pagina=0;pagina < procesoActivo->cantidadEntradasMemoria ; pagina++)
	{
		if(procesoActivo->paginasEnMemoria[pagina] != -1)
			return;
	}

	log_error(logger,"No hay marcos disponibles, se notifica al CPU");
	pthread_mutex_unlock(&mutexClock);
	*noHayMarcos = 1;

}


unsigned algoritmoclock(t_tablaDePaginas*procesoActivo,unsigned pagina,int *paginaSiYaEstabaEnMemoria)
{
	log_trace(loggerClock,"===========INICIO=CLOCK===================");
	unsigned punteroClock = procesoActivo->punteroClock;
	unsigned paginaApuntada;
	int marco = buscarMarcoDisponible();
	log_trace(loggerClock,"-Puntero clock:%d PID:%d",punteroClock,procesoActivo->pid);

	*paginaSiYaEstabaEnMemoria = paginaEnEntrada(pagina,procesoActivo);

	//La pagina a reemplazar ya estaba en la tabla de las paginas en memoria
	if(*paginaSiYaEstabaEnMemoria != -1 )
	{
		log_trace(loggerClock,"==============FIN=CLOCK==================\n");
		return punteroClock;
	}


	while(1)
	{

		//marco = buscarMarcoDisponible();
		paginaApuntada = procesoActivo->paginasEnMemoria[punteroClock];
		if(paginaApuntada == -1 )
		{
			if( marco != -1)
			{
			log_trace(loggerClock,"Encuentra un marco vacio");
			log_trace(loggerClock,"==============FIN=CLOCK==================\n");
			return punteroClock;
			}
			else
				punteroClock = avanzaPunteroClock(procesoActivo,punteroClock);
		}
		else
		{
			if(procesoActivo->entradaTablaPaginas[paginaApuntada].estaEnMemoria == 1)
			{
				log_trace(loggerClock,"Bit Presencia Pagina:%d se pone en 0",paginaApuntada);
				procesoActivo->entradaTablaPaginas[paginaApuntada].estaEnMemoria = 0;
				borrarEntradaTLB(procesoActivo->entradaTablaPaginas[paginaApuntada].marco);
				punteroClock = avanzaPunteroClock(procesoActivo,punteroClock);
			}
			else
			{
				log_trace(loggerClock,"==============FIN=CLOCK==================\n");
				return punteroClock;
			}

		 }
	}

	log_trace(loggerClock,"==============FIN=CLOCK==================\n");
	return punteroClock;
}


void actualizarTablaDePaginas(t_tablaDePaginas*procesoActivo)
{
	unsigned indice;
	buscarTablaSegun(procesoActivo->pid,&indice);
	pthread_mutex_lock(&mutexTablaPaginas);
	list_replace(tablasDePaginas,indice,procesoActivo);
	pthread_mutex_unlock(&mutexTablaPaginas);
	return;
}


unsigned actualizaPagina(unsigned pagina,t_tablaDePaginas* procesoActivo,int clienteUMC,int paginaEstabaEnMemoria)
{
	unsigned punteroClock = procesoActivo->punteroClock;
	unsigned paginaVieja;
	int marco;
	if(paginaEstabaEnMemoria == -1)
	{
		//Se actualiza la nueva pagina a las entradas en memoria
		paginaVieja = procesoActivo->paginasEnMemoria[punteroClock];
		if(paginaVieja != -1)
		{
			falloPagina(procesoActivo,paginaVieja);
			procesoActivo->entradaTablaPaginas[pagina].marco = procesoActivo->entradaTablaPaginas[paginaVieja].marco;
		}
		else
		{
			marco = buscarMarcoDisponible();
			procesoActivo->entradaTablaPaginas[pagina].marco = marco;
			marcoAsignadoA[marco] = procesoActivo->pid;
			marcoDisponible[marco] = -1;
		}

		procesoActivo->paginasEnMemoria[punteroClock] = pagina;
		procesoActivo->entradaTablaPaginas[pagina].estaEnMemoria = 1;
		procesoActivo->punteroClock = avanzaPunteroClock(procesoActivo,punteroClock);

		log_trace(loggerClock,"El puntero clock luego del algortimo:%d",procesoActivo->punteroClock);
	}
	else
		procesoActivo->entradaTablaPaginas[pagina].estaEnMemoria = 1;

	//Actualizar la tabla de paginas
	actualizarTablaDePaginas(procesoActivo);


	return procesoActivo->entradaTablaPaginas[pagina].marco;
}

void escribirEnMemoria(void* codigoPrograma,unsigned tamanioPrograma, unsigned pagina,t_tablaDePaginas*procesoActivo,int clienteUMC,int paginaEstabaEnMemoria)
{
	unsigned marco = actualizaPagina(pagina,procesoActivo,clienteUMC,paginaEstabaEnMemoria);
	if(paginaEstabaEnMemoria != -1)
		return;

	usleep(infoMemoria.retardo);
	pthread_mutex_lock(&mutexMemoria);
	memcpy(memoriaPrincipal + infoMemoria.tamanioDeMarcos*marco,codigoPrograma,tamanioPrograma);
	pthread_mutex_unlock(&mutexMemoria);
	free(codigoPrograma);
	return;
}
void mostrarTablaDePaginas(t_tablaDePaginas* procesoActivo)
{
	unsigned indice;
	log_trace(loggerClock,"======Tabla De Paginas PID:%d =====",procesoActivo->pid);
	log_trace(loggerClock,"------Paginas-----");
	for(indice = 0; indice < procesoActivo->cantidadEntradasTablaPagina; indice ++)
		log_trace(loggerClock,"Pagina:%d EstaEnMemoria:%d FueModificado:%d",
				indice,procesoActivo->entradaTablaPaginas[indice].estaEnMemoria,procesoActivo->entradaTablaPaginas[indice].fueModificado);

	log_trace(loggerClock,"===================================");
	return;
}

void algoritmoDeReemplazo(void* codigoPrograma,unsigned tamanioPrograma,unsigned pagina,t_tablaDePaginas* procesoActivo,int clienteUMC,int*noHayMarcos)
{
	pthread_mutex_lock(&mutexClock);
	unsigned punteroClock = 0;
	int paginaEstabaEnMemoria;
	unsigned marco = buscarMarcoDisponible();

	//Si no hay marcos disponibles
	if(marco == -1)
	{
		abortarSiNoHayPaginasEnMemoria(procesoActivo,clienteUMC,noHayMarcos);

		//nunca se carga el proceso a memoria
		if(*noHayMarcos == 1)
		{
			free(codigoPrograma);
			return;
		}
	}


	//Eleccion entre Algoritmos
	if(!strcmp("CLOCK",infoConfig.algoritmo))
		punteroClock = algoritmoclock(procesoActivo,pagina,&paginaEstabaEnMemoria);

	if(!strcmp("CLOCK-M",infoConfig.algoritmo))
		punteroClock = algoritmoClockMejorado(procesoActivo,pagina,&paginaEstabaEnMemoria);

	procesoActivo->punteroClock = punteroClock;


	//Escribe en memoria la nueva pagina que mando el SWAP
	escribirEnMemoria(codigoPrograma,tamanioPrograma,pagina,procesoActivo,clienteUMC,paginaEstabaEnMemoria);
	//mostrarTablaDePaginas(procesoActivo);
	pthread_mutex_unlock(&mutexClock);

	return ;
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

void traerPaginaAMemoria(unsigned pagina,t_tablaDePaginas* procesoActivo,int clienteUMC,int* noHayMarcos)
{
	t_mensaje aRecibir;

	//Pedimos pagina al SWAP
	log_trace(logger,"Pide al SWAP la pagina:%d pid:%d",procesoActivo->pid,pagina);
	pthread_mutex_lock(&mutexClientes);
	pedirPagAlSWAP(pagina,procesoActivo->pid);

	//Recibimos pagina del SWAP
	recibirMensaje(clienteSWAP, &aRecibir);
	pthread_mutex_unlock(&mutexClientes);

	//TEST
	if(pagina == paginaVariablesTest)
	{
		int offset;
		log_trace(loggerVariables,"Trae a Memoria Pagina:%d Tamaño:%d",pagina,aRecibir.head.tam_extra);
		for(offset = 0; offset < 9;offset=offset+4){
		void* codigoTest = malloc(4);
		memcpy(codigoTest,aRecibir.mensaje_extra+offset,4);
		log_trace(loggerVariables,"Var %d: int:%d",offset/4,*((int*)codigoTest));
		}
	}
	//--

	log_trace(logger,"Se Ejecuta el Algoritmo de reemplazo");
	algoritmoDeReemplazo(aRecibir.mensaje_extra,aRecibir.head.tam_extra,pagina,procesoActivo,clienteUMC,noHayMarcos);

	return;
}
void algoritmoLRU(t_entradaTLB *entradaTLB)
{
	log_trace(loggerTLB,"Se agrega entrada: pid:%d pagina:%d  marco:%d",entradaTLB->pid,entradaTLB->pagina,entradaTLB->marco);
	pthread_mutex_lock(&mutexTLB);
	int tamanioMaximoTLB = infoMemoria.entradasTLB;
	int tamanioTLB = list_size(TLB);
	if(tamanioTLB == tamanioMaximoTLB)
	{
		log_trace(loggerTLB,"Se borra la entradaTLB:%d (la menos usada)",tamanioMaximoTLB-1);
		log_trace(loggerTLB,"Se pone la nueva entradaTLB: PID:%d,Pagina:%d,Marco:%d al principio de la lista",entradaTLB->pid,entradaTLB->pagina,entradaTLB->marco);
		list_add_in_index(TLB,0,entradaTLB);
		list_remove_and_destroy_element(TLB,tamanioMaximoTLB,(void*)destruirTLB);
	}
	else{
		log_trace(loggerTLB,"Se pone la nueva entradaTLB: PID:%d,Pagina:%d,Marco:%d al principio de la lista",entradaTLB->pid,entradaTLB->pagina,entradaTLB->marco);
		list_add_in_index(TLB,0,entradaTLB);
	}
	pthread_mutex_unlock(&mutexTLB);

	return;
}

void actualizarTLB(t_entradaTablaPaginas entradaDePaginas,unsigned pagina,unsigned pidActual)
{
	if(infoMemoria.entradasTLB == 0)
		return;

	log_trace(loggerTLB,"Se agrega una entrada a la TLB");
	t_entradaTLB *entradaTLB = malloc(sizeof(t_entradaTLB));
	entradaTLB->pid = pidActual;
	entradaTLB->pagina = pagina;
	entradaTLB->estaEnMemoria = entradaDePaginas.estaEnMemoria;
	entradaTLB->fueModificado = entradaDePaginas.fueModificado;
	entradaTLB->marco         = entradaDePaginas.marco;
	log_trace(loggerTLB,"PID:%d",entradaTLB->pid);
	log_trace(loggerTLB,"Pagina:%d",entradaTLB->pagina);
	log_trace(loggerTLB,"Marco:%d",entradaTLB->marco);

	algoritmoLRU(entradaTLB);
	mostrarTLB();

	return;
}

int buscarEnTLB(unsigned paginaBuscada,unsigned pidActual)
{
	pthread_mutex_lock(&mutexTLB);
	int indice;
	int marco;
	t_entradaTLB *entradaTLB;

	if(infoMemoria.entradasTLB == 0)
	{
		pthread_mutex_unlock(&mutexTLB);
		return -1;
	}
	for(indice=0;indice < list_size(TLB);indice++)
	{
		entradaTLB = list_get(TLB,indice);
		if(entradaTLB->pid == pidActual && entradaTLB->pagina == paginaBuscada)
		{
			marco = entradaTLB->marco;
			log_trace(loggerTLB,"Se accedio a al indice: %d de la TLB",indice);
			log_trace(loggerTLB,"-PID:%d",entradaTLB->pid);
			log_trace(loggerTLB,"-Pagina:%d",entradaTLB->pagina);
			log_trace(loggerTLB,"-Marco :%d",entradaTLB->marco);
			log_trace(loggerTLB,"Se introduce la entradaTLB al inicio de la lista");
			//Sacar la entrada y ponerla inicio de la lista
			list_remove(TLB,indice);
			list_add_in_index(TLB,0,entradaTLB);
			pthread_mutex_unlock(&mutexTLB);
			mostrarTLB();
			return marco;
		}
	}

	log_trace(logger,"-TLB: miss");
	log_trace(logger1,"-TLB: miss");
	pthread_mutex_unlock(&mutexTLB);
	return -1;
}

void traducirPaginaAMarco(unsigned pagina,int *marco,t_tablaDePaginas*procesoActivo,int clienteUMC,int* noHayMarcos)
{
	//Buscar en TLB
	*marco = buscarEnTLB(pagina,procesoActivo->pid);
	if(*marco != -1)
	{
		log_trace(logger,"-TLB: hit -> Marco: %d",*marco);
		return;
	}
	//Buscar en tabla de paginas
	usleep(infoMemoria.retardo);
	if(procesoActivo->entradaTablaPaginas[pagina].estaEnMemoria == 1)
		{
			//Esta en memoria se copia el marco
			*marco = procesoActivo->entradaTablaPaginas[pagina].marco;
			log_trace(logger,"Pagina en Memoria:SI -> Marco:%d", *marco);
			log_trace(logger,"Se actualiza la TLB");
			actualizarTLB(procesoActivo->entradaTablaPaginas[pagina],pagina,procesoActivo->pid);
			return;
		}
		else
		{
			// Buscar en swap
			log_trace(logger,"Pagina en Memoria: NO");
			log_trace(logger,"Se trae la pagina del SWAP");
			traerPaginaAMemoria(pagina,procesoActivo,clienteUMC,noHayMarcos);
			*marco = procesoActivo->entradaTablaPaginas[pagina].marco;
			log_trace(logger,"PID:%d->Pagina:%d->Marco:%d",procesoActivo->pid,pagina,*marco);
			return;
		}



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
	log_trace(logger1,"============================");
	log_trace(logger1,"Procesos en Tablas:");
	for(proceso= 0; proceso < list_size(tablasDePaginas);proceso++)
	{
		tabla = list_get(tablasDePaginas,proceso);
		log_trace(logger1,"--------------------------------");
		log_trace(logger1,"Proceso PID: %d \n Paginas:%d \n PunteroClock:%d",tabla->pid,tabla->cantidadEntradasTablaPagina,tabla->punteroClock);
		log_trace(logger1,"Paginas:");

		for(pagina = 0;pagina < tabla->cantidadEntradasTablaPagina;pagina++)
		{
			if(tabla->entradaTablaPaginas[pagina].estaEnMemoria == 1)
				log_trace(logger1,"-Pagina:%d -> Marco:%d",pagina,tabla->entradaTablaPaginas[pagina].marco);
			else
				log_trace(logger1,"-Pagina:%d -> Marco:NULL",pagina);
		}
		log_trace(logger1,"--------------------------------");
	}
	log_trace(logger1,"============================");
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

unsigned copiarCodigo(unsigned paginaDondeEmpieza,unsigned paginasALeer,t_tablaDePaginas* procesoActivo,int clienteUMC,unsigned tamanio,unsigned offset,void* codigoAEnviar)
{
	unsigned offsetTest = offset; //TEST
	unsigned paginaATraducir;
	int marco;
	int noHayMarcos = 0;
	unsigned tamanioACopiar;
	if(paginasALeer == 1)
		tamanioACopiar = tamanio;
	else
		tamanioACopiar = infoMemoria.tamanioDeMarcos - offset;

	unsigned seLeyo = 0;

	if(paginaDondeEmpieza + paginasALeer <= procesoActivo->cantidadEntradasTablaPagina)
	{

		for(paginaATraducir = 0; paginaATraducir < paginasALeer;paginaATraducir++)
		{

			log_trace(logger,"Se traduce la Pagina: %d  Pid:%d al marco correspondiente" ,paginaDondeEmpieza+paginaATraducir,procesoActivo->pid);
			traducirPaginaAMarco(paginaDondeEmpieza+paginaATraducir,&marco,procesoActivo,clienteUMC,&noHayMarcos);
			if(noHayMarcos == 1)
				return 3;
			usleep(infoMemoria.retardo);
			pthread_mutex_lock(&mutexMemoria);
			memcpy(codigoAEnviar+seLeyo,memoriaPrincipal+infoMemoria.tamanioDeMarcos*marco+offset,tamanioACopiar);
			pthread_mutex_unlock(&mutexMemoria);


			offset = 0;
			seLeyo = seLeyo + tamanioACopiar;
			tamanio = tamanio - tamanioACopiar;
			//Se fija si esta por leer la ultima pagina
			if(paginaATraducir < paginasALeer -2)
			{
				tamanioACopiar = infoMemoria.tamanioDeMarcos;
			}
			else
			{
				tamanioACopiar = tamanio;
			}
		}
		log_trace(logger,"Se copia el codigo a enviar");
		usleep(infoMemoria.retardo);
		//TEST
		if(seLeyo == 4 && paginasALeer == 1)
			{
			log_trace(loggerVariables,"Envio Variable:Pagina %d var:%d",paginaDondeEmpieza,offsetTest/4);
			log_trace(loggerVariables,"Var en void*:%s",codigoAEnviar);
			log_trace(loggerVariables,"Var en int  :%d\n",*((int*)codigoAEnviar));
			}
		//--
		return 1;
	}
	else
	{
		log_error(logger,"Paginas fuera de segmento, se notifica al CPU");
		return 2;
	}
}


unsigned guardarCodigo(unsigned paginaDondeEmpieza,unsigned paginasALeer,t_tablaDePaginas*procesoActivo,int clienteUMC,unsigned tamanio,unsigned offset,void* codigoAGuardar)
{
	unsigned offsetTest = offset; //TEST
	unsigned paginaATraducir;
	int marco;
	int noHayMarcos=0;
	unsigned tamanioACopiar;
	unsigned seLeyo = 0;
	if(paginasALeer == 1)
		tamanioACopiar = tamanio;
	else
		tamanioACopiar = infoMemoria.tamanioDeMarcos - offset;


	if(paginaDondeEmpieza + paginasALeer <= procesoActivo->cantidadEntradasTablaPagina)
	{

		for(paginaATraducir = 0; paginaATraducir < paginasALeer;paginaATraducir++)
		{

			log_trace(logger,"Se traduce la pagina: %d con pid:%d al marco correspondiente \n" ,paginaDondeEmpieza+paginaATraducir,procesoActivo->pid);
			traducirPaginaAMarco(paginaDondeEmpieza+paginaATraducir,&marco,procesoActivo,clienteUMC,&noHayMarcos);
			if(noHayMarcos == 1)
				return 3;
			log_trace(logger,"Se copia el contenido en la pagina:%d pid:%d marco:%d",paginaDondeEmpieza+paginaATraducir,procesoActivo->pid,marco);
			usleep(infoMemoria.retardo);
			pthread_mutex_lock(&mutexMemoria);
			memcpy(memoriaPrincipal+infoMemoria.tamanioDeMarcos*marco+offset,codigoAGuardar+seLeyo,tamanioACopiar);
			pthread_mutex_unlock(&mutexMemoria);
			offset = 0;
			seLeyo = seLeyo + tamanioACopiar;
			tamanio = tamanio - tamanioACopiar;

			procesoActivo->entradaTablaPaginas[paginaDondeEmpieza+paginaATraducir].fueModificado = 1;

			//Se fija si esta por leer la ultima pagina
			if(paginaATraducir < paginasALeer -2)
				tamanioACopiar = infoMemoria.tamanioDeMarcos;
			else
				tamanioACopiar = tamanio;
		}
		actualizarTablaDePaginas(procesoActivo);

		//TEST
		if(seLeyo == 4 && paginasALeer == 1)
			{
			log_trace(loggerVariables,"Guardar Variable:Pagina %d var:%d",paginaDondeEmpieza,offsetTest/4);
			log_trace(loggerVariables,"Var en void*:%s",codigoAGuardar);
			log_trace(loggerVariables,"Var en int  :%d\n",*((int*)codigoAGuardar));
			}
		//--

		return 1;
	}
	else
	{
		log_error(logger,"Paginas fuera del stack, se notifica al CPU");
		return 2;
	}
}

void almacenarBytesEnPagina(t_mensaje mensaje,t_tablaDePaginas* procesoActivo, int clienteUMC)
{
	log_trace(logger,"=======Peticion de almacenaje de Variable ========");
	unsigned pagina  = mensaje.parametros[0];
	unsigned offset  = mensaje.parametros[1];
	unsigned tamanio = mensaje.parametros[2];
	void* codigo = malloc(tamanio);
	log_trace(logger,"-Pagina:%d",pagina);
	log_trace(logger,"-Offset:%d",offset);
	log_trace(logger,"-Tamano:%d",tamanio);
	memcpy(codigo,&mensaje.parametros[3],tamanio);


	unsigned paginasALeer = paginasARecorrer(offset,tamanio);

	//TEST
	if(paginasALeer == 1)
		paginaVariablesTest = pagina;
	//--

	unsigned estado = guardarCodigo(pagina,paginasALeer,procesoActivo,clienteUMC,tamanio,offset,codigo);

	enviarCodigoAlCPU(NULL,0,clienteUMC,estado);

	free(codigo);
	log_trace(logger,"====================================================");
	return;
}

void enviarBytesDeUnaPagina(t_mensaje mensaje,int clienteUMC,t_tablaDePaginas* procesoActivo)
{
	log_trace(logger,"=======Peticion de envio de codigo al CPU======");
	unsigned pagina  = mensaje.parametros[0];
	unsigned offset  = mensaje.parametros[1];
	unsigned tamanio = mensaje.parametros[2];
	unsigned paginasALeer = paginasARecorrer(offset,tamanio);
	void* codigoAEnviar = malloc(tamanio);
	log_trace(logger,"-Pagina:%d",pagina);
	log_trace(logger,"-Offset:%d",offset);
	log_trace(logger,"-Tamano:%d",tamanio);

	unsigned estado = copiarCodigo(pagina,paginasALeer,procesoActivo,clienteUMC,tamanio,offset,codigoAEnviar);


	log_trace(logger,"Se envia la instruccion al CPU");
	enviarCodigoAlCPU(codigoAEnviar,tamanio,clienteUMC,estado);
	log_trace(logger,"===============================================");

	free(codigoAEnviar);

	return;
}


void enviarTamanioDePagina(int clienteUMC)
{
	log_trace(logger,"Se envia el tamano de pagina");
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

void accionSegunCabecera(int clienteUMC)
{
	log_trace(logger,"Se creo un Hilo");
	unsigned pidActivo = 0;
	t_tablaDePaginas*procesoActivo = NULL;
	int cabeceraDelMensaje;
	t_mensaje mensaje;

	while(1){
	//	procesosEnTabla();
		if(recibirMensaje(clienteUMC,&mensaje) <= 0){
			clienteDesconectado(clienteUMC);
			freeMensaje(&mensaje);
		}
		usleep(infoMemoria.retardo);
		cabeceraDelMensaje = mensaje.head.codigo;
		switch(cabeceraDelMensaje){
			case INIT_PROG: inicializarPrograma(mensaje,clienteUMC);
				break;
			case FIN_PROG:  finPrograma(mensaje);
				break;
			case GET_DATA:  enviarBytesDeUnaPagina(mensaje,clienteUMC,procesoActivo);
				break;
			case GET_TAM_PAGINA: enviarTamanioDePagina(clienteUMC);
				break;
			case CAMBIO_PROCESO:
				procesoActivo = cambioProcesoActivo(mensaje.parametros[0],pidActivo);
				pidActivo = procesoActivo->pid;
				break;
			case RECORD_DATA:
				almacenarBytesEnPagina(mensaje,procesoActivo, clienteUMC);
				break;
		}
		freeMensaje(&mensaje);
	}
	return;
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

	printf("Esperando al Nucleo\n");
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
					pthread_create(&cliente, NULL, (void *)accionSegunCabecera, (void *) clienteUMC);
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
	t_tablaDePaginas *aux;
	tabla->pid = 2;
	list_add(hola,tabla);
	aux = list_remove(hola,0);
	free(aux);
	printf("hola");

	return 0;

}*/
