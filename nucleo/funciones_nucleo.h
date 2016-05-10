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

typedef struct PCB PCB;
struct PCB//Process Control Block
{
	unsigned int pid;//Process ID
	unsigned int pc; //Program Counter
	unsigned int sp; //Stack Pointer
	int state;//Estado del proceso
};

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

