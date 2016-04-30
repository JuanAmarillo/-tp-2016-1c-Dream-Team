#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int reconocerComando(char *comando)
{
	if(!strcmp(comando, "retardo")) return 1;
	if(!strcmp(comando, "dump"   )) return 2;
	if(!strcmp(comando, "flush"  )) return 3;

	return 0;//Error de Comando
}
void ejecutarComando(int id)//Ejecuta lo retornado por "reconocerComando"
{
	switch(id)
	{
		case 1:
			printf("Ejecutar \"retardo\"\n");
			break;
		case 2:
			printf("Ejecutar \"dump\"\n");
			break;
		case 3:
			printf("Ejecutar \"flush\"\n");
			break;
	}
}
