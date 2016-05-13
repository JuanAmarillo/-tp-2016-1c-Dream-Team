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
typedef struct {
	char *ip_nucleo;
	char *puerto_nucleo;
	char *ip_umc;
	char *puerto_umc;
} t_infoConfig;


/*
 * Variables Globales
 */
int socketUMC, socketNucleo;
t_infoConfig infoConfig;


/*
 * Funciones / Procedimientos
 */
void leerArchivoConfig();
int conectarseUMC();
int conectarseNucleo();
int enviarMensajeUMC(mensaje_t);
int enviarMensajeNucleo(mensaje_t);
int recibirMensajeUMC(mensaje_t *);
int recibirMensajeNucleo(mensaje_t *);
void testParser();
int crearConexion(const char *, const char *);


#endif /* CPU_H_ */
