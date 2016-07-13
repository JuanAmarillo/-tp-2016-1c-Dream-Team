/*
 * testUmc.c
 *
 *  Created on: 12/7/2016
 *      Author: utnso
 */
#include "umc.h"

void procesosEnTabla()
{
	unsigned proceso;
	t_tablaDePaginas *tabla;
	puts("\n");
	puts("Procesos en Tablas \n");
	for(proceso= 0; proceso < list_size(tablasDePaginas);proceso++)
	{
		tabla = list_get(tablasDePaginas,proceso);
		printf("Proceso pid: %d\n",tabla->pid);
	}

	return;
}
void testInicioPrograma(unsigned pid,unsigned paginas)
{
	char* codigoPrograma = malloc(4);
	t_mensaje inicioPrograma;
	inicioPrograma.head.codigo = INIT_PROG;
	inicioPrograma.head.cantidad_parametros = 2;
	inicioPrograma.parametros = malloc(2*sizeof(int));
	inicioPrograma.parametros[0] = pid;
	inicioPrograma.parametros[1] = paginas;
	inicioPrograma.head.tam_extra = sizeof(codigoPrograma);
	inicioPrograma.mensaje_extra = codigoPrograma;
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

/*
int main(){

	pthread_mutex_init(&mutexClientes,NULL);
	pthread_mutex_init(&mutexMemoria,NULL);
	pthread_mutex_init(&mutexTablaPaginas,NULL);
	pthread_mutex_init(&mutexClock,NULL);
	pthread_mutex_init(&mutexTLB,NULL);

	//Config
	leerArchivoConfig();
	inicializarEstructuras();
	//conectarAlSWAP();

	//servidor
	//gestionarConexiones();

	//test
	testInicioPrograma(3,4);
	testInicioPrograma(2,5);
	testFinPrograma(2);
	testInicioPrograma(4,1);
	testInicioPrograma(5,3);
	testFinPrograma(4);

	return 0;

}
*/
