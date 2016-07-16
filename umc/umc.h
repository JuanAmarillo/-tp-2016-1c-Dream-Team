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
	unsigned punteroClock;
	unsigned *paginasEnMemoria;
	unsigned cantidadEntradasTablaPagina;
	unsigned cantidadEntradasMemoria;
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
t_log* logger;
t_log* loggerConsola;
t_memoria infoMemoria;
t_infoConfig infoConfig;
int servidorUMC,clienteSWAP;
struct sockaddr_in direccionServidorUMC;
struct sockaddr_in direccionServidorSWAP;
fd_set master;
void* memoriaPrincipal;
t_list *tablasDePaginas;
t_list *TLB;
//t_entradaTLB  *entradaTLB;
pthread_mutex_t mutexClientes;
pthread_mutex_t mutexMemoria;
pthread_mutex_t mutexTablaPaginas;
pthread_mutex_t mutexTLB;

/*
 * Funciones
 */
void leerArchivoConfig();
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
unsigned cambioProcesoActivo(unsigned pid,unsigned pidActivo);
void inicializarPrograma(t_mensaje mensaje,int clienteUMC);
void eliminarDeMemoria(unsigned pid);
void finPrograma(t_mensaje finalizarProg);
void enviarPaginaAlSWAP(unsigned pagina,void* codigoDelMarco,unsigned pidActivo);
void falloDePagina(unsigned pidActivo);
void actualizarPagina(unsigned pagina,unsigned pidActivo);
void escribirEnMemoria(void* codigoPrograma,unsigned tamanioPrograma, unsigned pagina,unsigned pidActivo,unsigned punteroClock,unsigned indice);
unsigned algoritmoclock(unsigned pidActivo,unsigned *indice);
void pedirPagAlSWAP(unsigned pagina,unsigned pidActual);
void traerPaginaAMemoria(unsigned pagina,unsigned pidActual);
void actualizarTLB(t_entradaTablaPaginas entradaDePaginas,unsigned pagina,unsigned pidActual);
int buscarEnTLB(unsigned paginaBuscada,unsigned pidActual);
void traducirPaginaAMarco(unsigned pagina,int *marco,unsigned pidActual);
void almacenarBytesEnPagina(t_mensaje mensaje,unsigned pidActivo, int clienteUMC);
void enviarCodigoAlCPU(char* codigoAEnviar, unsigned tamanio,int clienteUMC);
void enviarBytesDeUnaPagina(t_mensaje mensaje,int clienteUMC,unsigned pidActual);
void enviarTamanioDePagina(int clienteUMC);
void accionSegunCabecera(int clienteUMC,unsigned pid);
void* gestionarSolicitudesDeOperacion(int clienteUMC);
int recibirConexiones();
int aceptarConexion();
int recibir(void *buffer,unsigned tamanioDelMensaje,int clienteUMC);
void conectarAlSWAP();
void enviar(void *buffer,int cliente);
void gestionarConexiones();



#endif /* UMC_H_ */
