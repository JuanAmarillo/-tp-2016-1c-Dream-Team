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
typedef struct PCB PCB;
typedef struct t_variable t_variable;
typedef struct t_posicionDeMemoria t_posicionDeMemoria;
typedef struct Nodo_argumento Nodo_argumento;
typedef struct Nodo_variable Nodo_variable;
typedef struct t_indice_stack t_indice_stack;
typedef struct t_indiceInstruccion t_indiceInstruccion;
typedef struct Nodo_indiceInstruccion Nodo_indiceInstruccion

//Indice de Stack//
//----------------------------------------------------------------
struct t_posicionDeMemoria
{
	unsigned int numeroPagina;
	unsigned int offset;
	size_t size;
};

struct t_variable
{
	char *identificador;
	t_posicionDeMemoria posicionMemoria;
};

struct Nodo_argumento//Nodo para hacer una lista de argumentos
{
	t_posicionDeMemoria argumento;//El argumento se representa con su direccion de memoria
	Nodo_argumento *sgte;
};

struct Nodo_variable//Nodo para hacer una lista de variables
{
	t_variable variable;
	Nodo_variable *sgte;
};

struct t_indice_stack//Esto va al PCB
{
	Nodo_argumento *args;
	Nodo_variable *vars;
	unsigned int retPos;
	t_posicionDeMemoria retVar;
};
//-----------------------------------------------------------------

//Indice de Instruccion
//-----------------------------------------------------------------
struct t_indiceInstruccion
{
	unsigned int offset_inicio;
	unsigned int offset_fin;
};

struct Nodo_indiceInstruccion//Nodo para hacer una lista de indices de instruccion
{
	t_indiceInstruccion indice_instruccion;
	Nodo_indiceInstruccion *sgte;
};
typedef t_indiceCodigo Nodo_indiceInstruccion*;//Esto va al PCB (lista de Nodos de "indices de instruccion", formando el "indice de codigo")
//------------------------------------------------------------------

//Indice de Etiquetas
//------------------------------------------------------------------
struct t_indiceEtiquetas
{
	//serializacion... (por favor apruebeme xD)
};
//------------------------------------------------------------------


//PCB
//------------------------------------------------------------------
struct PCB//Process Control Block
{
	unsigned int pid;//Process ID
	unsigned int pc; //Program Counter
	unsigned int sp; //Stack Pointer
	int state;//Estado del proceso
	t_indiceCodigo indice_codigo;
	t_indiceEtiquetas indice_etiquetas;
	t_indiceStack indice_stack;
};
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

