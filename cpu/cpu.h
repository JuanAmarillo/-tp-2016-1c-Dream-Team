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

/*
 * Funciones / Procedimientos
 */
void leerArchivoConfig();
int conectarseUMC();
int conectarseNucleo();
int enviarMensajeUMC(t_mensaje);
int enviarMensajeNucleo(t_mensaje);
int recibirMensajeUMC(t_mensaje *);
int recibirMensajeNucleo(t_mensaje *);
int crearConexion(const char *, const char *);
void signal_sigusr1(int);
char *obtenerSiguienteIntruccion();
unsigned obtenerTamanoPaginasUMC();
void enviarPCBnucleo();
int recibirQuantum();


#endif /* CPU_H_ */
