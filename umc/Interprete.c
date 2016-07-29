#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <commons/string.h>
#include <commons/collections/list.h>
#include "umc.h"

int reconocerComando(char *comando);
void limpiarPaginas();
void limpiarTLB();
void flushDeConsola(char* parametroExtra);
void retardoDeConsola(char* parametroExtra);
void mostrarProceso(t_tablaDePaginas *tabla);
void mostrarContenidoDePaginas(t_tablaDePaginas *tl);
void mostrarContenido(int marco);
void mostrar(t_tablaDePaginas* tl);
void mostrarTodoDeConsola();
void mostrarUnoDeConsola(int pid);
void ejecutarComando(char *parametroExtra, int accion);
void instrucciones();
void accion();


int reconocerComando(char *comando) {
	char* accion = string_split(comando, " ")[0];
	if (!strcmp(accion, "retardo"))
		return 1;
	if (!strcmp(accion, "dump"))
		return 2;
	if (!strcmp(accion, "flush"))
		return 3;
	if(!strcmp(accion,"help"))
		return 4;
	return 0; //Error de Comando
}

void limpiarPaginas() {
	pthread_mutex_lock(&mutexTablaPaginas);
	int cantidadDeTablas = list_size(tablasDePaginas);
	t_tablaDePaginas* tablaPagina;
	int indice,entrada;
	for (indice = 0; indice < cantidadDeTablas; indice++)
	{

		tablaPagina = list_get(tablasDePaginas, indice);
		for(entrada=0 ; entrada < tablaPagina->cantidadEntradasTablaPagina;entrada++)
		{
			tablaPagina->entradaTablaPaginas[entrada].fueModificado = 1;
		}
	}
	pthread_mutex_unlock(&mutexTablaPaginas);
}

void limpiarTLB() {
	pthread_mutex_lock(&mutexTLB);
	list_clean(TLB);
	pthread_mutex_unlock(&mutexTLB);
}

void flushDeConsola(char* parametroExtra) {
	if (!strcmp(parametroExtra, "tlb\n")) {
		log_trace(logger, "Se limpia la TLB por comando de la consola");
		log_trace(logger1, "Se va a proceder a limpiar la TLB");
		limpiarTLB();
		log_trace(logger, "Se limpio la TLB");
		log_trace(logger1, "Se limpio la TLB");
	} else {
		log_trace(logger, "Se van a marcar todas las paginas como modificadas");
		log_trace(logger1, "Se van a poner todas las paginas como modificadas");
		limpiarPaginas();
		log_trace(logger1, "Se modificaron las paginas");
		log_trace(logger, "Se modificaron las paginas");
	}
}

void retardoDeConsola(char* parametroExtra) {
	unsigned retardo = atoi(parametroExtra);
	log_trace(logger, "El retardo se cambio de %u unidades a %u unidades",infoMemoria.retardo, retardo);
	log_trace(logger1, "El retardo se cambio de %u unidades a %u unidades",infoMemoria.retardo, retardo);
	infoMemoria.retardo = retardo;
}

void mostrarProceso(t_tablaDePaginas* tabla) {
	unsigned pagina;
	log_trace(logger1, "==============Tabla De Paginas PID:%d===============================",tabla->pid);
	for (pagina = 0; pagina < tabla->cantidadEntradasTablaPagina; pagina++) {
		if (tabla->entradaTablaPaginas[pagina].estaEnMemoria == 1)
			log_trace(logger1,"Pagina:%d EstaEnMemoria:%d FueModificado:%d -> Marco:%d",
					pagina,tabla->entradaTablaPaginas[pagina].estaEnMemoria,tabla->entradaTablaPaginas[pagina].fueModificado,tabla->entradaTablaPaginas[pagina].marco);
		else
			log_trace(logger1,"Pagina:%d EstaEnMemoria:%d FueModificado:%d -> Marco:NULL",
								pagina,tabla->entradaTablaPaginas[pagina].estaEnMemoria,tabla->entradaTablaPaginas[pagina].fueModificado,tabla->entradaTablaPaginas[pagina].marco);
	}
	log_trace(logger1, "===================================================================");
	return;
}

void mostrarContenidoDePaginas(t_tablaDePaginas* tl) {
	int i = 0;
	log_trace(logger1, "Proceso N° %u", tl->pid);
	log_trace(logger1, "El contenido de las paginas es:");
	while (i < tl->cantidadEntradasTablaPagina)
	{
		if (tl->entradaTablaPaginas[i].estaEnMemoria)
		{
			log_trace(logger1,"El contenido de la pagina nro %i del proceso es:");
			mostrarContenido(tl->entradaTablaPaginas[i].marco);
		}
		i++;
	}
	log_trace(logger1, "============================");
}

void mostrarContenido(int marco) {
	void* contenido = malloc(infoMemoria.tamanioDeMarcos);
	pthread_mutex_lock(&mutexMemoria);
	memcpy(contenido,memoriaPrincipal+marco*infoMemoria.tamanioDeMarcos,infoMemoria.tamanioDeMarcos);
	pthread_mutex_unlock(&mutexMemoria);
	log_trace(logger1, "%s", contenido);
	free(contenido);
}

void mostrar(t_tablaDePaginas* tl) {
	mostrarProceso(tl);
	mostrarContenidoDePaginas(tl);
}

void mostrarTodoDeConsola() {
	pthread_mutex_lock(&mutexTablaPaginas);
	int tamanioLista = list_size(tablasDePaginas);
	int i = 0;
	t_tablaDePaginas* tl;
	while (i < tamanioLista) {
		tl = (t_tablaDePaginas*) list_get(tablasDePaginas, i);
		mostrar(tl);
		i++;
	}
	pthread_mutex_unlock(&mutexTablaPaginas);
}

void mostrarUnoDeConsola(int pid) {
	t_tablaDePaginas* tl = NULL;
	unsigned indice;
	unsigned seEncontro;
	tl= buscarTabla(pid,&indice,&seEncontro);
	if(seEncontro == 1)
		mostrar(tl);
	else
		log_trace(logger1,"El Pid %d no es un proceso valido",pid);
}

void ejecutarComando(char *parametroExtra, int accion) //Ejecuta lo retornado por "reconocerComando"
{
	int pid;
	char** parametros = string_split(parametroExtra," ");
	switch (accion) {
	case 1:
		retardoDeConsola(parametros[1]);
		break;
	case 2:
		pid = atoi(parametros[1]);
		if (pid == 0){
			mostrarTodoDeConsola();
		}
		else{
			mostrarUnoDeConsola(pid);
		}
		break;
	case 3:
		flushDeConsola(parametros[1]);
		break;
	case 4:
		instrucciones();
		break;
	default:
		log_trace(logger1, "La accion no es contemplada");
		log_trace(logger, "La accion no es contemplada");
		break;
	}
	free(parametroExtra);
	return;
}

void instrucciones() {
	puts("Por favor, introduzca un comando perteneciente a los siguientes:");
	puts("1) Introducir \"retardo\" seguido de un numero para modificar el retardo de espera\n");
	puts("2) Introducir \"dump\" seguido de un 0 para mostrar todos los procesos, o el PID de un proceso en memoria\n");
	puts("3) Introducir \"flush\" seguido de:");
	puts("-----\"tlb\" para limpiar el contenido de la TLB");
	puts("-----\"memory\" para marcar todas las pagtinas como modificadas\n");
}

void accion(){
	instrucciones();
	char* instruccion = NULL;
	unsigned int len = 20;
	unsigned decision;
	while(1){
		getline(&instruccion,&len,stdin);
		decision = reconocerComando(instruccion);
		ejecutarComando(instruccion,decision);
		instruccion = NULL;
	}
}
void crearHiloInterprete()
{
	pthread_t interprete;
	pthread_create(&interprete, NULL, (void *)accion,NULL);
	return;
}


int main(){


	//Config
	leerArchivoConfig();
	inicializarEstructuras();
	conectarAlSWAP();

	//servidor
	//crearHiloInterprete();
	gestionarConexiones();

	return 0;
}

