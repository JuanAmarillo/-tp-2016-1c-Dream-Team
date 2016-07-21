#include "E-S.h"

t_PCB* atenderProceso(t_dispositivo *dispositivo)
{
	t_PCB *ret;

	escribirLog("%s: Se atendera el proceso %d (cantidad de operaciones: %d)\n", dispositivo->nombre, dispositivo->atendiendo->proceso->pid, dispositivo->atendiendo->cantOp);

	while(dispositivo->atendiendo->cantOp > 0)
	{
		usleep(dispositivo->io_sleep * 1000);
	//	sleep(3);
		dispositivo->atendiendo->cantOp --;
	}

	ret = dispositivo->atendiendo->proceso;
	dispositivo->atendiendo = NULL;

	return ret;
}

void planificarDispositivo(t_dispositivo *dispositivo)
{
	escribirLog("Se comienza a administrar el dispositivo: %s\n", dispositivo->nombre);

	t_PCB *actual;

	while(1)
	{
		//espera activa por procesos bloqueados
		while(queue_is_empty(dispositivo->cola));

		dispositivo->atendiendo = queue_pop(dispositivo->cola);

		actual = atenderProceso(dispositivo);
		escribirLog("Se atendio al proceso %d y se pondra listo otra vez\n", actual->pid);
		FD_CLR(actual->pid, &conjunto_procesos_bloqueados);
		ponerListo(actual);
	}
}

void* llamar_planificarDispositivo(void *data)
{
	t_dispositivo *dispositivo = (t_dispositivo*) data;

	planificarDispositivo(dispositivo);

	return NULL;
}
