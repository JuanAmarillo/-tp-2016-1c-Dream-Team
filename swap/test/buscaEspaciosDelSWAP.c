
/*
 * prueba.c
 *
 *  Created on: 15/5/2016
 *      Author: utnso
 */
#include <commons/bitarray.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
/*
int main1(){
	char data[] = {0b01100011,0,0b01010101};
	t_bitarray *bitarray = bitarray_create(data, sizeof(data));
	printf("%d\n",sizeof(data));
	int i=0;
	for (i=0;i<24;i++){
		if(bitarray_test_bit(bitarray,i))
			printf("Es un uno!\n");
		else
			printf("Es un cero!\n");
	}

    return 0;
}

int main2(){

	char *pagDisp = malloc(4* sizeof(char));
	printf("%a\n",pagDisp);
	strcpy((char *)*pagDisp,"ssssssss\0");

	printf("%s\n %a \n",pagDisp, pagDisp);
	free(pagDisp);
	return 0;
}

int main3(){
	char* data = malloc(20);
	printf("%s\n",data);
	strcpy(data,"456\n");
	printf("%s\n",data);
	free(data);
}
*/
unsigned searchSpaceToFill(unsigned programSize, t_bitarray* DISP_PAGINAS){
	int CANTIDAD_PAGINAS = 6;
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
void prueba1(){
	int CANTIDAD_PAGINAS = 6;
	int tamanio = (CANTIDAD_PAGINAS/8)+1;
	char *data = malloc(tamanio);
	strcpy(data,"\0");
	t_bitarray* DISP_PAGINAS = bitarray_create(data,tamanio);
	printf("%d\n",searchSpaceToFill(1,DISP_PAGINAS));
}

void prueba2(){
	int CANTIDAD_PAGINAS = 6;
	int tamanio = (CANTIDAD_PAGINAS/8)+1;
	char *data = malloc(tamanio);
	strcpy(data,"\0");
	t_bitarray* DISP_PAGINAS = bitarray_create(data,tamanio);
	bitarray_set_bit(DISP_PAGINAS,1);
	printf("%d\n",searchSpaceToFill(2,DISP_PAGINAS));
}

void prueba3(){
	int CANTIDAD_PAGINAS = 6;
	int tamanio = (CANTIDAD_PAGINAS/8)+1;
	char *data = malloc(tamanio);
	strcpy(data,"\0");
	t_bitarray* DISP_PAGINAS = bitarray_create(data,tamanio);
	bitarray_set_bit(DISP_PAGINAS,1);
	bitarray_set_bit(DISP_PAGINAS,3);
	bitarray_set_bit(DISP_PAGINAS,5);
	printf("%d\n",searchSpaceToFill(2,DISP_PAGINAS));
}

void prueba4(){
	int CANTIDAD_PAGINAS = 6;
	int tamanio = (CANTIDAD_PAGINAS/8)+1;
	char *data = malloc(tamanio);
	strcpy(data,"\0");
	t_bitarray* DISP_PAGINAS = bitarray_create(data,tamanio);
	bitarray_set_bit(DISP_PAGINAS,1);
	bitarray_set_bit(DISP_PAGINAS,2);
	bitarray_set_bit(DISP_PAGINAS,3);
	bitarray_set_bit(DISP_PAGINAS,4);
	bitarray_set_bit(DISP_PAGINAS,5);
	printf("%d\n",searchSpaceToFill(2,DISP_PAGINAS));
}

int main(){
	prueba1(); //DEVUELVE 0
	prueba2(); //DEVUELVE 2
	prueba3(); //DEVUELVE -1
	prueba4(); //DEVUELVE -2
	return 0;
}
