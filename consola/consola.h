#ifndef CONSOLA_H_
#define CONSOLA_H_

#define NUEVO_PROGRAMA 100

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

t_mensaje codigo_to_mensaje(char* codigo)
{
	unsigned int tamCod = strlen(codigo)+1;
	t_mensajeHead head = {NUEVO_PROGRAMA, 0, tamCod};
	t_mensaje mensaje;
	mensaje.head = head;
	mensaje.parametros = NULL;
	mensaje.mensaje_extra = malloc(tamCod);
	memcpy(mensaje.mensaje_extra, codigo, tamCod);
	memset(mensaje.mensaje_extra + tamCod - 1, '\0', 1);
	return mensaje;
}

 /*
 * leerArchivoConfig();
 * Parametros: -
 * Descripcion: Procedimiento que lee el archivo config.conf y lo carga en la variable infoConfig
 * Return: -
 */
void leerArchivoConfig();

 /*
- * inicializarDireccionNucleo();
- *Parámetros: -
- *Descripción: Procedimiento que inicializa la estructura sockaddr_in con los valores levantados de config.conf
- *Return: -
-*/
void inicializarDireccionNucleo();

#endif
