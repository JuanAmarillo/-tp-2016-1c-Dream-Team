#ifndef FUNCIONES_NUCLEO_H_
#define FUNCIONES_NUCLEO_H_

//Bibliotecas a usar
#include <commons/config.h>
#include <commons/string.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/time.h>
#include <unistd.h>

//Cuántas conexiones se aceptarán
#define BACKLOG 100
/*
 * Estructuras de datos
 */
typedef struct t_infoConfig t_infoConfig;
struct t_infoConfig
{
	char *puerto_prog;
	char *puerto_cpu;
	char *puerto_umc;
};

/*****************************************************************************************/
//__________________________________INICIO PCB___________________________________________//

typedef struct {
	unsigned numeroPagina;
	unsigned offset;
	size_t size;
} t_posicionDeMemoria;

typedef struct {
	char *identificador;
	t_posicionDeMemoria posicionMemoria;
} t_variable;

typedef struct {
	t_list *args;
	t_list *vars;
	unsigned retPos;
	t_posicionDeMemoria retVar;
} t_indice_stack;

typedef struct {
	char *etiqueta;
	unsigned int pc_instruccion;
} t_indiceEtiqueta;

typedef struct {
	unsigned offset_inicio;
	unsigned offset_fin;
} t_indiceCodigo;

typedef {
	unsigned pid;
	unsigned pc;
	unsigned sp;
	unsigned paginas_codigo;
	unsigned estado;
	t_indiceCodigo *indice_codigo;
	t_dictionary *indice_etiquetas;
	t_list *indice_stack;
} struct t_PCB;
//___________________________________FIN PCB_____________________________________________//
/*****************************************************************************************/

/*Variables Globales*/
/*----------------------------------------------------------*/
t_infoConfig infoConfig; //archivo de configuración

struct sockaddr_in direccionParaConsola, direccionParaCPU, direccionUMC; //Direcciones propia
struct sockaddr_in direccionCliente;//direccion del cliente

//file descriptor para escuchar (listener), para una nueva conexión (new) y para explorar conexiones (explorer)
int fd_umc, fd_listener_consola, fd_listener_cpu, fd_new, fd_explorer;

//buffers para datos recibidos de los clientes
char bufferConsola[100], bufferCPU[100];


#endif /* FUNCIONES_NUCLEO_H_ */

