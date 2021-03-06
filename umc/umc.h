/*
 * umc.h
 *
 *  Created on: 27/4/2016
 *      Author: utnso
 */

#ifndef UMC_H_
#define UMC_H_

/*
 * Bibliotecas
 */
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <commons/config.h>
#include <commons/string.h>
#include <commons/collections/list.h>
#include <commons/log.h>
#include "messageCode.h"
#include "protocolo_mensaje.h"

/*
 * Estructuras de datos
 */
typedef struct{
	char *ip;
	char *puertoUMC;
	char *puertoSWAP;
	char *algoritmo;
} t_infoConfig;

typedef struct{
	int marcos;
	int tamanioDeMarcos;
	int maxMarcosPorPrograma;
	int entradasTLB;
	int retardo;
} t_memoria;


typedef struct{
	unsigned estaEnMemoria;
	unsigned fueModificado;
	unsigned marco;
} t_entradaTablaPaginas;

typedef struct{
	unsigned pid;
	int *paginasEnMemoria;
	unsigned cantidadEntradasTablaPagina;
	unsigned cantidadEntradasMemoria;
	unsigned punteroClock;
	t_entradaTablaPaginas *entradaTablaPaginas;
} t_tablaDePaginas;

typedef struct{
	unsigned pid;
	unsigned pagina;
	unsigned marco;
	unsigned estaEnMemoria;
	unsigned fueModificado;
}t_entradaTLB;



/*
 * Variables Globales
 */
int* marcoDisponible;
int* marcoAsignadoA;
t_log* logger;
t_log* logger1;
t_log* loggerTLB;
t_log* loggerVariables;
t_log* loggerClock;
t_memoria infoMemoria;
t_infoConfig infoConfig;
int servidorUMC,clienteSWAP,clienteNucleo;
int paginaVariablesTest;
struct sockaddr_in direccionServidorUMC;
struct sockaddr_in direccionServidorSWAP;
fd_set master;
void* memoriaPrincipal;
t_list *tablasDePaginas;
t_list *TLB;
//t_entradaTLB  *entradaTLB;
pthread_mutex_t mutexClientes;
pthread_mutex_t mutexMemoria;
pthread_mutex_t mutexClock;
pthread_mutex_t mutexTablaPaginas;
pthread_mutex_t mutexTLB;

/*
 * Funciones
 */
void leerArchivoConfig();
void procesosEnTabla();
struct sockaddr_in setDireccion(const char *puerto);
void inicializarEstructuras();
void clienteDesconectado(int clienteUMC);
void pedirReservaDeEspacio(unsigned pid,unsigned paginasSolicitadas);
void empaquetarYEnviar(t_mensaje mensaje,int clienteUMC);
void enviarProgramaAlSWAP(unsigned pid, unsigned paginasSolicitadas,unsigned tamanioCodigo, char* codigoPrograma);
void enviarNoHaySuficienteEspacio(int clienteUMC);
unsigned enviarCodigoAlSwap(unsigned paginasSolicitadas,char* codigoPrograma,unsigned pid,unsigned tamanioCodigo,int clienteUMC);
void crearTablaDePaginas(unsigned pid,unsigned paginasSolicitadas);
void borrarEntradasTLBSegun(unsigned pidActivo);
t_tablaDePaginas* cambioProcesoActivo(unsigned pid,unsigned pidActivo);
void inicializarPrograma(t_mensaje mensaje,int clienteUMC);
void eliminarDeMemoria(unsigned pid,unsigned* huboFallo);
void finPrograma(t_mensaje finalizarProg);
void enviarPaginaAlSWAP(unsigned pagina,void* codigoDelMarco,unsigned pidActivo);
void falloDePagina(unsigned pidActivo);
void actualizarPagina(unsigned pagina,unsigned pidActivo);
void escribirEnMemoria(void* codigoPrograma,unsigned tamanioPrograma, unsigned pagina,t_tablaDePaginas*procesoActivo,int clienteUMC,int paginaEstabaEnMemoria);
unsigned algoritmoclock(t_tablaDePaginas*procesoActivo,unsigned pagina,int *paginaSiYaEstabaEnMemoria);
void pedirPagAlSWAP(unsigned pagina,unsigned pidActual);
void traerPaginaAMemoria(unsigned pagina,t_tablaDePaginas* procesoActivo,int clienteUMC,int* noHayMarcos);
void actualizarTLB(t_entradaTablaPaginas entradaDePaginas,unsigned pagina,unsigned pidActual);
int buscarEnTLB(unsigned paginaBuscada,unsigned pidActual);
void traducirPaginaAMarco(unsigned pagina,int *marco,t_tablaDePaginas*procesoActivo,int clienteUMC,int* noHayMarcos);
void almacenarBytesEnPagina(t_mensaje mensaje,t_tablaDePaginas* procesoActivo, int clienteUMC);
void enviarCodigoAlCPU(char* codigoAEnviar, unsigned tamanio,int clienteUMC,unsigned estado);
void enviarBytesDeUnaPagina(t_mensaje mensaje,int clienteUMC,t_tablaDePaginas* procesoActivo);
void enviarTamanioDePagina(int clienteUMC);
void accionSegunCabecera(int clienteUMC);
void* gestionarSolicitudesDeOperacion(int clienteUMC);
int recibirConexiones();
int aceptarConexion();
int recibir(void *buffer,unsigned tamanioDelMensaje,int clienteUMC);
void conectarAlSWAP();
void enviar(void *buffer,int cliente);
void copiarInstruccion(int *marco,unsigned paginasALeer,unsigned tamanio,unsigned offset,void* codigoAEnviar);
void gestionarConexiones();



#endif /* UMC_H_ */
