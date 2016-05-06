#ifndef NUCLEO_H_
#define NUCLEO_H_

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

/*
 * Funciones / Procedimientos
 */
void leerArchivoConfig(void);// Lee archivo config.conf
void inicializarDirecciones(void);//Inicializa las direcciones propias para escuchar y la dirección de la UMC
void conectar_a_umc(void);//Conectarse al proceso UMC
void abrirPuertos(void);//Pone en modo escucha los sockets asociados a los puertos para Consola y para CPU
int maximofd(int, int);//Compara valor numérico entre dos file descriptors y devuelve el máximo
void administrarConexiones(void);//Contiene todos los procedimientos para recibir datos de los otros procesos

#endif /* NUCLEO_H_ */

