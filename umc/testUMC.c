/*
 * testUMC.c
 *
 *  Created on: 1/7/2016
 *      Author: utnso
 */
#include "umc.h"

void testLeerArchivoConfig()
{
	printf("--------------------------------------");
	printf("TestLeerArchivoConfig\n");
	leerArchivoConfig();
	printf("El IP         : %s\n",infoConfig.ip);
	printf("El PuertoUMC  : %s\n",infoConfig.puertoUMC);
	printf("El PuertoSWAP : %s\n",infoConfig.puertoSWAP);
	printf("Marcos               : %d\n",infoMemoria.marcos);
	printf("TamañoMarcos         : %d\n",infoMemoria.maxMarcosPorPrograma);
	printf("MaxMarcosPorPrograma : %d\n",infoMemoria.maxMarcosPorPrograma);
	printf("entradasTLB          : %d\n",infoMemoria.entradasTLB);
	printf("--------------------------------------");
}
void testInicializarPrograma()
{

}
int main()
{	logger = log_create("UMC_TEST.txt", "UMC", 1, LOG_LEVEL_TRACE);
	testLeerArchivoConfig();
	testInicializarPrograma();
	inicializarEstructuras();
	return 0;
}

