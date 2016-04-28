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
int enviarMensajeUMC(char *mensaje);
int enviarMensajeNucleo(char *mensaje);
int recibirMensajeUMC(char *mensaje);
int recibirMensajeNucleo(char *mensaje);
void testParser();
int crearConexion(const char *ip, const char *puerto);
int enviarMensaje(int serverSocket, char *mensaje);
int recibirMensaje(int serverSocket, char *mensaje, unsigned tamano);


#endif /* CPU_H_ */
