/*
 * cpu.h
 *
 *  Created on: 23/4/2016
 *      Author: utnso
 */

#ifndef CPU_H_
#define CPU_H_

/*
 * Estructuras de datos
 */
typedef struct{
	char *ip_nucleo;
	char *puerto_nucleo;
	char *ip_umc;
	char *puerto_umc;
} t_infoConfig;

typedef struct{
	int page;
	int offset;
	int size;
} t_posicionMemoria;

typedef struct{
	char *id;
	t_posicionMemoria localizacion;
} t_vars;

typedef int t_indiceCodigo[2];

typedef struct{
	char *id;
	int pc;
} t_indiceEtiquetas;

typedef struct{
	t_list *args; 						// Lista del tipo t_posicionMemoria
	t_list *vars; 						// Lista del tipo t_vars
	int retPos;
	t_posicionMemoria retVar;
} t_indiceStack;

typedef struct{
	int id;
	int pc;
	t_list *indiceCodigo; 				// Lista del tipo t_indiceCodigo
	t_dictionary *indiceEtiquetas; 		// Diccionario del tipo t_indiceEtiquetas
	t_list *indiceStack; 				// Lista del tipo t_indiceStack
} t_pcbData;

/*
 * Variables Globales
 */
t_infoConfig infoConfig;


/*
 * Funciones / Procedimientos
 */
void leerArchivoConfig();
int conectarseUMC();
int conectarseNucleo();
void testParser();
int crearConexion(const char *ip, const char *puerto);


#endif /* CPU_H_ */
