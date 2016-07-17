/*
 * testUmc.c
 *
 *  Created on: 12/7/2016
 *      Author: utnso
 */
#include "umc.h"

void testInicioPrograma(unsigned pid,unsigned paginas)
{
	char* hola = malloc(paginas*4);
	int i= 0;
	for(i=0;i<paginas-1;i++)
		memcpy(hola+i*4,"hola",4);
	memcpy(hola+paginas*3,"hol\0",4);
	t_mensaje inicioPrograma;
	inicioPrograma.head.codigo = INIT_PROG;
	inicioPrograma.head.cantidad_parametros = 2;
	inicioPrograma.parametros = malloc(2*sizeof(int));
	inicioPrograma.parametros[0] = pid;
	inicioPrograma.parametros[1] = paginas;
	inicioPrograma.head.tam_extra = 16;
	inicioPrograma.mensaje_extra = hola;
	printf("%s",inicioPrograma.mensaje_extra);
	inicializarPrograma(inicioPrograma,4);
	procesosEnTabla();

	return;
}

void testFinPrograma(unsigned pid)
{
	puts("\n");
	printf("Se procede a eliminar el proceso con pid: %d\n",pid);
	t_mensaje finaPrograma;
	finaPrograma.head.codigo = FIN_PROG;
	finaPrograma.head.cantidad_parametros = 1;
	finaPrograma.parametros = malloc(sizeof(int));
	finaPrograma.parametros[0] = pid;
	finaPrograma.head.tam_extra = 0;
	finaPrograma.mensaje_extra = NULL;
	finPrograma(finaPrograma);
	procesosEnTabla();

	return;

}
void testSolicitarBytesDeUnaPagina(unsigned pagina, unsigned offset, unsigned tamano,unsigned pid)
{
	t_mensaje bytesPagina;
	bytesPagina.head.codigo = GET_TAM_PAGINA;
	bytesPagina.head.cantidad_parametros = 3;
	bytesPagina.parametros = malloc(sizeof(int)*3);
	bytesPagina.head.tam_extra = 0;
	bytesPagina.mensaje_extra = NULL;
	enviarBytesDeUnaPagina(bytesPagina,4,pid);
	procesosEnTabla();
	return;
}

/*
int main(){

	pthread_mutex_init(&mutexClientes,NULL);
	pthread_mutex_init(&mutexMemoria,NULL);
	pthread_mutex_init(&mutexTablaPaginas,NULL);
	pthread_mutex_init(&mutexTLB,NULL);

	//Config
	leerArchivoConfig();
	inicializarEstructuras();
	conectarAlSWAP();

	//servidor
	//gestionarConexiones();

	//test
	testInicioPrograma(1,4);
	testInicioPrograma(2,5);
	procesosEnTabla();
	traerPaginaAMemoria(0,1);
	procesosEnTabla();
	traerPaginaAMemoria(0,2);
	procesosEnTabla();
	traerPaginaAMemoria(1,1);
	procesosEnTabla();
	traerPaginaAMemoria(1,2);
	procesosEnTabla();
	traerPaginaAMemoria(3,1);
	procesosEnTabla();
	traerPaginaAMemoria(2,1);
	procesosEnTabla();
	traerPaginaAMemoria(4,2);
	procesosEnTabla();
	traerPaginaAMemoria(0,1);
	procesosEnTabla();
	traerPaginaAMemoria(1,1);
	procesosEnTabla();
	traerPaginaAMemoria(3,1);
	procesosEnTabla();



	return 0;

}*/

