#include "nucleo.h"

//ACORDATE DE QUE FALTAN LOS MUTEX
int main(void)
{
	leerArchivoConfig();
	
	inicializarDirecciones();
	
	conectar_a_umc();

	abrirPuertos();

	inicializarListas();

	montarHilos();

	return EXIT_SUCCESS;
}
