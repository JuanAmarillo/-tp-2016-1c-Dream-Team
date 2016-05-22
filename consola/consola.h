#ifndef CONSOLA_H_
#define CONSOLA_H_

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
