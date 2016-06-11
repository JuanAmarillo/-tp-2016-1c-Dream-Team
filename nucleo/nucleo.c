#include "nucleo.h"

//ACORDATE DE QUE FALTAN LOS MUTEX
int main(int numArgs, char**argsVec)
{
	leerArchivoConfig(argsVec[1]);
	
	crearLog();

	inicializarDirecciones();
	
	conectar_a_umc();

	recibirTamPaginas();

	abrirPuertos();

	inicializarListas();

	montarHilos();

	return EXIT_SUCCESS;
}
