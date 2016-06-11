#ifndef NUCLEO_5_NUCLEO_H_
#define NUCLEO_5_NUCLEO_H_

//Bibliotecas a usar
#include <commons/config.h>
#include <commons/string.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "archivoLog.h"
#include "planificador.h"

/*
 * Funciones / Procedimientos
 */
void leerArchivoConfig(char*);// Lee archivo config.conf
void inicializarDirecciones(void);//Inicializa las direcciones propias para escuchar y la dirección de la UMC
void inicializarListas(void);//Inicializa cola de listos, bloquados, y demas conjuntos
void conectar_a_umc(void);//Conectarse al proceso UMC
void abrirPuertos(void);//Pone en modo escucha los sockets asociados a los puertos para Consola y para CPU
int maximofd(int, int);//Compara valor numérico entre dos file descriptors y devuelve el máximo
void administrarConexiones(void);//Contiene todos los procedimientos para recibir datos de los otros procesos
void montarHilos(void);//Crea, inicializa, corre, y espera a que terminen los tres hilos principales (administrador de conexiones, planificador e interfaz de usuario)
void recibirTamPaginas(void);//Recibe el tamaño de pagina en la umc

#endif /* NUCLEO_5_NUCLEO_H_ */

