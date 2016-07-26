#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <commons/string.h>
#include <commons/collections/list.h>
#include "umc.h"
/*
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
	return 0; //Error de Comando
}

void limpiarPaginas() {
	int cantidadEntradas = list_size(tablasDePaginas);
	t_tablaDePaginas* t;
	int i = 0, j;
	for (i = 0; i < cantidadEntradas; i++) {
		j = 0;
		t = (t_tablaDePaginas*) list_get(tablasDePaginas, i);
		while (j < t->cantidadEntradasTablaPagina) {
			t->entradaTablaPaginas->fueModificado = 1;
			j++;
		}
	}
}

void limpiarTLB() {
	list_clean(TLB);
}

void flushDeConsola(char* parametroExtra) {
	printf("La diferencia entre %s y %s", parametroExtra,"tlb");
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
		log_trace(logger1, "Se limpiaron las paginas");
		log_trace(logger, "Se limpiaron las paginas");
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
	log_trace(logger, "============================");
	log_trace(logger, "Procesos en Tablas ");
	log_trace(logger,"Proceso pid: %d, paginas:%d, estan en memoria:%d", tabla->pid, tabla->cantidadEntradasTablaPagina);
	log_trace(logger, "Paginas en memoria:");
	for (pagina = 0; pagina < tabla->cantidadEntradasTablaPagina; pagina++) {
		if (tabla->entradaTablaPaginas[pagina].estaEnMemoria == 1)
			log_trace(logger, "Pagina:%d -> Marco:%d\n", pagina,tabla->entradaTablaPaginas[pagina].marco);
	}
	log_trace(logger, "============================");
	return;
}

void mostrarContenidoDePaginas(t_tablaDePaginas* tl) {
	int i = 0;
	log_trace(logger1, "Proceso N° %u", tl->pid);
	log_trace(logger1, "El contenido de las paginas es:");
	while (i < tl->cantidadEntradasTablaPagina) {
		if (tl->entradaTablaPaginas->estaEnMemoria) {
			log_trace(logger1,"El contenido de la pagina nro %i del proceso es:");
			mostrarContenido(tl->entradaTablaPaginas->marco);
		} else {
			log_trace(logger1,
					"La pagina nro %u del proceso no esta en memoria", i);
		}
		i++;
	}
}

void mostrarContenido(int marco) {
	char* contenido = malloc(infoMemoria.tamanioDeMarcos);
	contenido = string_substring((char*) memoriaPrincipal, marco*infoMemoria.tamanioDeMarcos, infoMemoria.tamanioDeMarcos);
	log_trace(logger1, "%s", *contenido);
	free(contenido);
}

void mostrar(t_tablaDePaginas* tl) {
	mostrarProceso(tl);
	mostrarContenidoDePaginas(tl);
}

void mostrarTodoDeConsola() {
	int tamanioLista = list_size(tablasDePaginas);
	int i = 0;
	t_tablaDePaginas* tl;
	while (i < tamanioLista) {
		tl = (t_tablaDePaginas*) list_get(tablasDePaginas, i);
		mostrar(tl);
		i++;
	}
}

void mostrarUnoDeConsola(int pid) {
	unsigned* indice = malloc(sizeof(unsigned));
	*indice = 0;
	puts("Despues del indice");
	t_tablaDePaginas* tl = buscarTablaSegun(pid, indice);
	mostrar(tl);
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
/*
int main(){
	leerArchivoConfig();
	inicializarEstructuras();
	crearTablaDePaginas(1,2);
	crearTablaDePaginas(2,2);
	char* contenido = malloc(5);
	strcpy(memoriaPrincipal,"abcdefghijklmnopqrstuvwxyz");
	puts("strcpy");
	contenido = string_substring((char*) memoriaPrincipal, 10, 5);
	puts("substring");
	printf("Substring: %s", contenido);
	accion();

	return 0;

}

*/
