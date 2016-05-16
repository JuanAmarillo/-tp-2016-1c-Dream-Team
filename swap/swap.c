/*
 * swap.c
 *
 *  Created on: 26/4/2016
 *      Author: utnso
 */
#include <commons/config.h>
#include <commons/string.h>
#include <commons/bitarray.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include "swap.h"

int main(){
	readConfigFile();
	crearArchivoSWAP();
	crearEstructurasDeManejo();
	setSocket();
	bindSocket();
	acceptSocket();
	accionesDeFinalizacion();
	return 0;
}
// INICIO DE ESTRUCTURA OBLIGATORIA DEL PROCESO SWAP
void readConfigFile(){
	t_config *config = config_create("config.conf");
		if (config == NULL) {
			free(config);
			printf("1");
			abort();
		}
	PUERTO_ESCUCHA = atoi(config_get_string_value(config, "PUERTO_ESCUCHA"));
	NOMBRE_SWAP = config_get_string_value(config, "NOMBRE_SWAP");
	CANTIDAD_PAGINAS= atoi(config_get_string_value(config, "CANTIDAD_PAGINAS"));
	TAMANIO_PAGINA= atoi(config_get_string_value(config, "TAMANIO_PAGINA"));
	RETARDO_COMPACTACION= atoi(config_get_string_value(config, "RETARDO_COMPACTACION"));
}

void crearArchivoSWAP(){
	char comandoCreacion[100];
	sprintf(comandoCreacion, "dd if=/dev/zero of=%s bs=%i count=%i", NOMBRE_SWAP, TAMANIO_PAGINA, CANTIDAD_PAGINAS);
	if (system(comandoCreacion)){
		printf("No se pudo crear el archivo %s\n", NOMBRE_SWAP);
		exit(1);
	}
	SWAPFILE= fopen(NOMBRE_SWAP, "r+");
}

void crearEstructurasDeManejo(){
	int tamanio = (CANTIDAD_PAGINAS/8)+1;
	char *data = malloc(tamanio);
	strcpy(data,"\0");
	DISP_PAGINAS = bitarray_create(data,tamanio);
	INFO_PROG = malloc(tamanio*sizeof(t_infoProg));
	limpiarI_P(tamanio);
}

void limpiarI_P(int tamanio){
	int i=0;
	while (i<tamanio){
		INFO_PROG[i].LONGITUD = 0;
		INFO_PROG[i].PAG_INICIAL = 0;
		INFO_PROG[i].PID = 0;
		i++;
	}
}

void setSocket(){
		myAddress.sin_family = AF_INET;
		myAddress.sin_port = htons(PUERTO_ESCUCHA);
		myAddress.sin_addr.s_addr = INADDR_ANY;
		memset(myAddress.sin_zero, '\0', sizeof(myAddress.sin_zero));
}

void bindSocket(){
	int auxiliar=1;
	listeningSocket=socket(AF_INET, SOCK_STREAM, 0);
	setsockopt(listeningSocket, SOL_SOCKET, SO_REUSEADDR, &auxiliar, sizeof(int));
	bind(listeningSocket,(struct sockaddr*) &myAddress, sizeof(struct sockaddr));
}

void acceptSocket() {
	struct sockaddr_in addr;	// Esta estructura contendra los datos de la conexion del cliente. IP, puerto, etc.
	socklen_t addrlen = sizeof(addr);
	listen(listeningSocket,10);
	int socketCliente = accept(listeningSocket, (struct sockaddr *) &addr, &addrlen);
	printf("Se ha conectado al UMC\n");
}

void accionesDeFinalizacion() {
	fclose(SWAPFILE);
	bitarray_destroy(DISP_PAGINAS);
	free(INFO_PROG);
}
// FIN DE ESTRUCTURA OBLIGATORIA DEL PROCESO SWAP
// INICIO DE MANEJO DE PAGINAS DEL PROCESO SWAP
void assignPage(unsigned nroPag, estructuraAGuardar* str){
	fwrite(&str,TAMANIO_PAGINA,1,SWAPFILE);
	bitarray_set_bit(DISP_PAGINAS, nroPag);
}

void unAssignPage(unsigned nroPag){
	bitarray_clean_bit(DISP_PAGINAS,nroPag);
}

estructuraAGuardar* getPage(unsigned nroPag){
	estructuraAGuardar* pagOrigen;
	fread(&pagOrigen,TAMANIO_PAGINA,1,SWAPFILE);
	bitarray_clean_bit(DISP_PAGINAS,nroPag);
	return pagOrigen;
}
// FIN DE MANEJO DE PAGINAS DEL PROCESO SWAP

unsigned searchSpaceToFill(unsigned programSize){
	int freeSpace =0; 			//PARA REALIZAR COMPACTACION
	int freeSpaceInARow=0;		//PARA ASIGNAR SIN COMPACTAR
	int counter=0;				//CONTADOR DE PAGINAS
	while(counter<CANTIDAD_PAGINAS){
		if(bitarray_test_bit(DISP_PAGINAS, counter)!=0){
			freeSpaceInARow=0;
		}
		else{
			freeSpace++;
			freeSpaceInARow++;
			if(programSize<=freeSpaceInARow){
				return (counter-freeSpaceInARow+1); //DEVUELVE EL NRO DE PAGINA DONDE INICIA EL SEGMENTO LIBRE PARA ASIGNAR EL PROGRAMA
			}
		}
		counter++;
	}
	if(programSize<=freeSpace){
		return -1;
	}
	return -2;
}
