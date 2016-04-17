#ifndef CONSOLA_H_
#define CONSOLA_H_

/*
 * Estructuras de datos
 */
typedef struct{
	char * ip;
	char * puerto;
} t_infoConfig;

/*
 * Variables Globales
 */
t_infoConfig infoConfig;


/*
 * Funciones / Procedimientos
 */
void leerArchivoConfig();

#endif
