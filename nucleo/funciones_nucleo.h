#ifndef NUCLEO_5_FUNCIONES_NUCLEO_H_
#define NUCLEO_5_FUNCIONES_NUCLEO_H_

//Bibliotecas a usar
#include <commons/config.h>
#include <commons/string.h>
#include <commons/collections/list.h>
#include <stdio.h>
//#include <stdlib.h> ya está en planificador.h
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/time.h>
#include <unistd.h>
#include <pthread.h>
#include "archivoLog.h"
#include "interfaz.h"
#include "planificador.h"
#include "variables_compartidas.h"
#include "semaforos.h"

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
	char *quantum;
	char *quantum_sleep;
	char **array_dispositivos;
	char **array_io_sleeps;
	char **array_variables_compartidas;
	char **array_sem_id;
	char **array_sem_init;
	char *stack_size;
	char* ip_umc;
};

/*Variables Globales*/
/*----------------------------------------------------------*/
t_infoConfig infoConfig; //archivo de configuración

char rutaArchivoConfig[100];

struct sockaddr_in direccionParaConsola, direccionParaCPU, direccionUMC; //Direcciones propia
struct sockaddr_in direccionCliente;//direccion de cualquier cliente que se conecte

//file descriptor para escuchar (listener), para una nueva conexión (new) y para explorar conexiones (explorer)
int fd_umc, fd_listener_consola, fd_listener_cpu, fd_new, fd_explorer;
unsigned int tamPaginas;
unsigned int stack_size;

//buffers para datos recibidos de los clientes
t_mensaje mensajeConsola, mensajeCPU, mensajeUMC;

fd_set conjunto_pids_abortados;

//FUNCIONES
void estado_to_string(int estado, char *string);
unsigned int mensaje_to_tamPag(t_mensaje*);
int enviarInfoUMC(unsigned int pid, unsigned int cantidadPaginas, const char *codigo);
#endif /* NUCLEO_5_FUNCIONES_NUCLEO_H_ */

