#include "nucleo.h"
int main(void)
{
	// Leer archivo config.conf
	leerArchivoConfig();

	//Inicializar nuestra direccion
	inicializarDirecciones();

	//Conectarse al proceso UMC
	conectar_a_umc();

	//Poner en modo escucha los sockets asociados a los puertos para Consola y para CPU
	abrirPuertos();

	administrarConexiones();

	return EXIT_SUCCESS;
}
