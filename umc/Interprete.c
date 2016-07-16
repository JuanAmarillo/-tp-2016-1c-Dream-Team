#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int reconocerComando(char *comando)
{
	char* accion = string_split(comando, " ")[0];
	if(!strcmp(accion, "retardo")) return 1;
	if(!strcmp(accion, "dump"   )) return 2;
	if(!strcmp(accion, "flush"  )) return 3;

	return 0;//Error de Comando
}

void limpiarPaginas(){
	int cantidadEntradas = list_size(tablasDePaginas);
	t_tablaDePaginas t;
	int i = 0, j;
	for(i = 0; i<cantidadEntradas ; i++){
		j=0;
		t = list_get(tablasDePaginas,i);
		while(j<t.cantidadEntradasTablaPagina){
			t.entradaTablaPaginas->fueModificado = 1;
			j++;
		}
	}
}

void limpiarTLB() {
	list_clean(TLB);
}

void flushDeConsola(char* parametroExtra) {
	if (!strcmp(*parametroExtra, "tlb")) {
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

void retardoDeConsola(char* parametroExtra){
		unsigned retardo = itoa(*parametroExtra)
		log_trace(logger,"El retardo se cambio  de %u unidades a %u unidades", infoMemoria.retardo ,retardo);
		log_trace(logger1,"El retardo se cambio  de %u unidades a %u unidades", infoMemoria.retardo ,retardo);
		infoMemoria.retardo = retardo;
}

void mostrarProceso(t_tablaDePaginas tabla){
		unsigned proceso;
		unsigned pagina;
		log_trace(logger,"============================");
		log_trace(logger,"Procesos en Tablas ");
		log_trace(logger,"Proceso pid: %d, paginas:%d, estan en memoria:%d,el punteroClock en: %d ",tabla->pid,tabla->cantidadEntradasTablaPagina,tabla->punteroClock);
		log_trace(logger,"Paginas en memoria:");
		for(pagina = 0;pagina < tabla->cantidadEntradasTablaPagina;pagina++)
			{
				if(tabla->entradaTablaPaginas[pagina].estaEnMemoria == 1)
					log_trace(logger,"Pagina:%d -> Marco:%d\n",pagina,tabla->entradaTablaPaginas[pagina].marco);
			}
		log_trace(logger,"============================");
		return;
}

void mostrarContenidoDePaginas(t_tablaDePaginas tl){
	int i=0;
	log_trace(logger1,"Proceso N° %u",tl.pid);
	log_trace(logger1,"El contenido de las paginas es:");
	while(i<tl.cantidadEntradasTablaPagina){
		if(tl.entradaTablaPaginas->estaEnMemoria){
			log_trace(logger1,"El contenido de la pagina nro %i del proceso es:");
			mostrarContenido(tl.entradaTablaPaginas->marco);
		}
		else{
			log_trace(logger1,"La pagina nro %u del proceso no esta en memoria", i);
		}
		i++;
	}
}

void mostrarContenido(int marco){
	   char* contenido = malloc(infoMemoria.tamanioDeMarcos);
	   strncpy(contenido,(char* ) memoriaPrincipal[marco*infoMemoria.tamanioDeMarcos],infoMemoria.tamanioDeMarcos);
	   log_trace(logger1,"%s",*contenido);
	   free(contenido);
}

void mostrar(t_tablaDePaginas tl) {
	mostrarProceso(tl);
	mostrarContenidoDePaginas(tl);
}

void mostrarTodoDeConsola(){
	int tamanioLista = list_size(tablasDePaginas);
	int i= 0;
	while(i<tamanioLista){
		t_tablaDePaginas tl = list_get(tablasDePaginas,i);
		mostrar(tl);
	}
}

void mostrarUnoDeConsola(int pid){
	unsigned* indice;
	*indice = 0;
	t_tablaDePaginas* tl = buscarTablaSegun(pid,indice);
	t_tablaDePaginas tabla = *tl;
	mostrar(tl);
}

void ejecutarComando(char *parametroExtra, int accion) //Ejecuta lo retornado por "reconocerComando"
{
	switch(accion)
	{
		case 1:
			retardoDeConsola(parametroExtra);
			break;
		case 2:
			int pid = string_split(parametroExtra," ");
			if(pid == 0)
				mostrarTodoDeConsola();
			else
				mostrarUnoDeConsola(pid);
			break;
		case 3:
			flushDeConsola(parametroExtra);
			break;
		default:
			log_trace(logger1,"La accion no es contemplada");
			log_trace(logger,"La accion no es contemplada");
			break;
	}
	free(parametroExtra);
	return;
}
