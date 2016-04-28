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
//Puerto de el propio núcleo
#define MIPUERTO 8080
/*
 * Estructuras de datos
 */
typedef struct{
	char *puerto_prog;
	char *puerto_cpu;
	char *puerto_umc;
} t_infoConfig;


/*Variables Globales*/
/*----------------------------------------------------------*/
t_infoConfig infoConfig; //archivo de configuración

struct sockaddr_in direccionParaConsola, direccionParaCPU, direccionUMC; //Dirección propia
struct sockaddr_in direccionCliente;//direccion del cliente

//file descriptor para escuchar (listener), para una nueva conexión (new) y para explorar conexiones (explorer)
int fd_umc, fd_listener_consola, fd_listener_cpu, fd_new, fd_explorer;



char bufferConsola[100], bufferCPU[100];//buffers para datos recibidos de los clientes

/*
 * Funciones / Procedimientos
 */
void leerArchivoConfig(void);
void inicializarDirecciones(void);
void conectar_a_umc(void);
void abrirPuertos(void);
int maximofd(int, int);
void administrarConexiones(void);
#endif /* NUCLEO_H_ */

