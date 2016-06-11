/*
 * testSwap.h
 *
 *  Created on: 10/6/2016
 *      Author: utnso
 */

#ifndef TESTSWAP_H_
#define TESTSWAP_H_

int inicializar();
int limpiar();
void agregarTests(CU_pSuite prueba);
int main();
void testNoHayEspacioParaAlmacenar();
void testHayQueCompactar();
void testDevuelvePosicion2();
void testDevuelvePosicion0();
void testSeCorreElProgramaDentroDelArchivo();

#endif /* TESTSWAP_H_ */
