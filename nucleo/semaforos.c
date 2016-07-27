#include "semaforos.h"

int semaforo_wait(t_PCB *proceso, t_semaforo *s)
{
	s->cuenta--;

	if(s->cuenta < 0)
	{
		if(!proceso)
		{
			perror("Wait: wait() detecto que el proceso que decremento el semaforo debia ser bloqueado, sin embargo el argumento recibido en el parametro \"proceso\" fue NULL (revisar mecanismo de prediccion de wait)");
			abort();
		}

		queue_push(s->cola, proceso);
		bloquear(proceso, NULL, 0);
		return 0;
	}

	else return 1;
}

void semaforo_signal(t_semaforo *s)
{
	s->cuenta++;

	if(s->cuenta <= 0)
	{
		t_PCB *proceso;
		proceso = queue_pop(s->cola);

		//Eliminar de la cola de bloquados
		eliminarProcesoSegunPID(cola_bloqueados->elements, proceso->pid);

		ponerListo(proceso);
	}
}

t_semaforo* nombre_to_semaforo(const char *nombre)
{
	int i;
	for(i = 0; i < cantidadSemaforos; ++i)
	{
		if(!strcmp(semaforos[i].id, nombre))
			return &semaforos[i];
	}
	return NULL;
}

int existeSemaforo(const char *nombre)
{
	int i;
	for(i = 0; i < cantidadSemaforos; ++i)
		if(!strcmp(nombre, semaforos[i].id))
			return 1;
	return 0;
}
