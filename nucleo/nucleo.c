#include "nucleo.h"
int main(void)
{
	leerArchivoConfig();
	
	inicializarDirecciones();
	
	conectar_a_umc();

	abrirPuertos();

	administrarConexiones();

	return EXIT_SUCCESS;
}
