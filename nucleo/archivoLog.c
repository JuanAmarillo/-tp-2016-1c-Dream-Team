#include "archivoLog.h"

void crearLog(void)
{
	archivoLog = fopen("Archivo de Log.txt", "wt");
	fclose(archivoLog);
}

int escribirLog(const char* format, ...)
{
	pthread_mutex_lock(&mutex_log);

	archivoLog = fopen("Archivo de Log.txt", "at");

	va_list arg;
	int done;

	va_start (arg, format);
	done = vfprintf (archivoLog, format, arg);
	va_end (arg);

	fclose(archivoLog);

	pthread_mutex_unlock(&mutex_log);

	return done;
}

static void *imprimirPidLog(void *pcb)
{
	escribirLog("%d\n", ((t_PCB*)pcb)->pid);
	return NULL;
}

void mostrarColaPorLog(const t_queue* cola)
{
	list_map(cola->elements, imprimirPidLog);
}
