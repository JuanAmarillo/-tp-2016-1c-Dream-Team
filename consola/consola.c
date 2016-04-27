#include <commons/config.h>
#include <commons/string.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>

struct sockadrr_in direccionNucleo;

int main(int argc, char** argv){

	// Leer archivo config.conf
	leerArchivoConfig();
	//inicializar estructura de socket con los datos del nucleo
	inicializarDireccionNucleo();
	int miSocket = socket(PF_INET, SOCK_STREAM, 0);
	if (connect (miSocket, (sockaddr*) &direccionNucleo, sizeof(sockaddr_in)) == -1) { 
		printf("Error de connect\n");
		exit(1);
	}
	char mensaje[5] = "Hola";
	send (miSocket, mensaje, 5, 0); //para mensaje también se podría usar strlen(mensaje)+1
	printf ("Conectado al servidor. Bienvenido al sistema, ya puede enviar mensajes. Escriba 'exit' para salir\n");
	int enviar = 1;
	while (enviar) {
		fgets (mensaje, 5, stdin);
		if (!strcmp (mensaje, "exit\n")) enviar = 0;
		if (enviar) send (miSocket, mensaje, strlen(mensaje) +1, 0);
	}
	close (miSocket);
	return EXIT_SUCCESS;
}


/*
 * leerArchivoConfig();
 * Parametros: -
 * Descripcion: Procedimiento que lee el archivo config.conf y lo carga en la variable infoConfig
 * Return: -
 */
void leerArchivoConfig() {

	t_config *config = config_create("config.conf");

	if (config == NULL) {
		free(config);
		abort();
	}
	// Guardo los datos en una variable global
	infoConfig.ip = config_get_string_value(config, "IP");
	infoConfig.puerto = config_get_string_value(config, "PUERTO");

	// No uso config_destroy(config) porque bugea
	free(config->path);
	free(config);
}

/*
 * inicializarDireccionNucleo();
 *Parámetros: -
 *Descripción: Procedimiento que inicializa la estructura sockaddr_in con los valores levantados de config.conf
 *Return: -
*/
void inicializarDireccionNucleo (){

	direccionNucleo.sin_family = AF_INET;
	direccionNucleo.sin_port = htons (atoi(infoConfig.puerto));
	direccionNucleo.sin_addr.s_addr = inet_addr(infoConfig.ip);
	direccionNucleo.sin_zero = memset (direccionNucleo.sin_zero, '\0', sizeof (direccionNucleo.sin_zero));

}
