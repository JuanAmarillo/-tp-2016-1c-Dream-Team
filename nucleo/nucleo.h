/*
 * nucleo.h
 *
 *  Created on: 17/4/2016
 *      Author: utnso
 */

#ifndef NUCLEO_H_
#define NUCLEO_H_

//Cuántas conexiones se aceptarán
#define BACKLOG 100

/*
 * Estructuras de datos
 */
typedef struct{
	char *puerto_prog;
	char *puerto_cpu;
	char *puerto_umc;
} t_infoConfig;

/*
 * Variables Globales
 */
t_infoConfig infoConfig;


/*
 * Funciones / Procedimientos
 */
void leerArchivoConfig();

#endif /* NUCLEO_H_ */
