#include "nucleo.h"
int main(void)
{
	leerArchivoConfig();
	
	inicializarDirecciones();
	
	conectar_a_umc();

	abrirPuertos();

	inicializarListas();

	administrarConexiones();

	return EXIT_SUCCESS;
}
