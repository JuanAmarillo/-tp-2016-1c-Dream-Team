#include "E-S.h"

void atenderProceso(t_dispositivo *dispositivo)
{
	while(dispositivo->atendiendo > 0)
	{
		usleep(dispositivo->io_sleep * 1000);
		dispositivo->atendiendo->cantOp --;
	}

	dispositivo->atendiendo = NULL;
}

void planificarDispositivo(t_dispositivo *dispositivo)
{
	while(1)
	{
		//espera activa por procesos bloqueados
		while(queue_is_empty(dispositivo->cola));

		dispositivo->atendiendo = queue_pop(dispositivo->cola);

		atenderProceso(dispositivo);
	}
}

void* llamar_planificarDispositivo(void *data)
{
	t_dispositivo *dispositivo = (t_dispositivo*) data;

	planificarDispositivo(dispositivo);

	return NULL;
}
