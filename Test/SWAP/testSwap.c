/*
 * testSwap.c
 *
 *  Created on: 9/6/2016
 *      Author: utnso
 */

#include "CUnit/Basic.h"
#include "../../swap/swap.h"
#include <commons/collections/list.h>
#include "testSwap.h"
int inicializar() {
    readConfigFile();
	unlink(SWAPFILE);
    crearArchivoSWAP();
    bitarray_destroy(DISP_PAGINAS);
	int tamanio = (CANTIDAD_PAGINAS/8)+1;
	char *data = malloc(tamanio);
	strcpy(data,"\0");
	t_bitarray* DISP_PAGINAS = bitarray_create(data,tamanio);
    return SWAPFILE != NULL ? 0 : -1;
}

int limpiar() {
    accionesDeFinalizacion();
    return unlink(SWAPFILE);

}

void agregarTests(CU_pSuite prueba){
	  CU_add_test(prueba, "En 0", testDevuelvePosicion0);
	  CU_add_test(prueba, "En 2", testDevuelvePosicion2);
	  CU_add_test(prueba, "Compactar", testHayQueCompactar);
	  CU_add_test(prueba, "Rechazar", testNoHayEspacioParaAlmacenar);
}

int main() {
  CU_initialize_registry();
  CU_pSuite prueba = CU_add_suite("Suite de prueba", inicializar, limpiar);
  agregarTests(prueba);
  CU_basic_set_mode(CU_BRM_VERBOSE);
  CU_basic_run_tests();
  CU_cleanup_registry();

  return CU_get_error();
}

void testNoHayEspacioParaAlmacenar(){

	bitarray_set_bit(DISP_PAGINAS,1);
	bitarray_set_bit(DISP_PAGINAS,2);
	bitarray_set_bit(DISP_PAGINAS,3);
	bitarray_set_bit(DISP_PAGINAS,4);
	bitarray_set_bit(DISP_PAGINAS,5);
	printf("%s","Pruebo que no hay espacio para almacenar");
	CU_ASSERT_EQUAL(searchSpace(2,DISP_PAGINAS),-2);
}

void testHayQueCompactar(){
	int CANTIDAD_PAGINAS = 6;
	int tamanio = (CANTIDAD_PAGINAS/8)+1;
	char *data = malloc(tamanio);
	strcpy(data,"\0");
	t_bitarray* DISP_PAGINAS = bitarray_create(data,tamanio);
	bitarray_set_bit(DISP_PAGINAS,1);
	bitarray_set_bit(DISP_PAGINAS,3);
	bitarray_set_bit(DISP_PAGINAS,5);
	CU_ASSERT_EQUAL(searchSpace(2,DISP_PAGINAS),-1);
}

void testDevuelvePosicion2(){
	int CANTIDAD_PAGINAS = 6;
	int tamanio = (CANTIDAD_PAGINAS/8)+1;
	char *data = malloc(tamanio);
	strcpy(data,"\0");
	t_bitarray* DISP_PAGINAS = bitarray_create(data,tamanio);
	bitarray_set_bit(DISP_PAGINAS,1);
	CU_ASSERT_EQUAL(searchSpace(2,DISP_PAGINAS),2);
}

void testDevuelvePosicion0(){
	void prueba1(){
	int CANTIDAD_PAGINAS = 6;
	int tamanio = (CANTIDAD_PAGINAS/8)+1;
	char *data = malloc(tamanio);
	strcpy(data,"\0");
	t_bitarray* DISP_PAGINAS = bitarray_create(data,tamanio);
	CU_ASSERT_EQUAL(searchSpace(2,DISP_PAGINAS),0);
	}
}

void testSeCorreElProgramaDentroDelArchivo(){

}

