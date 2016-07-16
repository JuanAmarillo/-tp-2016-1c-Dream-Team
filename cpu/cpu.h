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
int notificacion_signal_sigusr1 = 0; // Bandera de señal SIGUSR1
t_infoConfig infoConfig;
t_PCB pcb_global;
int estado_ejecucion; // 0- Todo OK; 1- Fin del Programa ;
t_log* logger;
unsigned tamano_pagina_umc;

/*
 * Funciones / Procedimientos
 */
void leerArchivoConfig();
int conectarseUMC();
int conectarseNucleo();
void enviarMensajeUMC(t_mensaje);
void enviarMensajeNucleo(t_mensaje);
void recibirMensajeUMC(t_mensaje *);
void recibirMensajeNucleo(t_mensaje *);
int crearConexion(const char *, const char *);
void signal_sigusr1(int);
char *obtenerSiguienteIntruccion();
unsigned obtenerTamanoPaginasUMC();
void enviarPCBnucleo(unsigned);
void recibirQuantum(int *, int *);
void notificarCambioProceso();


#endif /* CPU_H_ */
