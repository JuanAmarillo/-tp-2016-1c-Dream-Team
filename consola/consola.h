#ifndef CONSOLA_H_
#define CONSOLA_H_

#include "protocolo_mensaje.h"

/*
 * Estructuras de datos
 */
typedef struct{
	char *ip;
	char *puerto;
} t_infoConfig;

/*
 * Variables Globales
 */
t_infoConfig infoConfig;


/*
 * Funciones / Procedimientos
 */

int crearConexion(const char *ip, const char *puerto);
t_mensaje codigo_to_mensaje(char* codigo);
void leerArchivoConfig();

#endif
