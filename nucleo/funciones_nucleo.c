#include "funciones_nucleo.h"

/*
 * leerArchivoConfig();
 * Parametros: -
 * Descripcion: Procedimiento que lee el archivo config.conf y lo carga en la variable infoConfig
 * Return: -
 */
void leerArchivoConfig(char *ruta)
{
	char *ruta_aux = (char*) malloc(100);
	if(ruta)
		strcpy(ruta_aux, ruta);
	else
	{
		printf("No se ingresó la ruta del archivo de configuracion, ingresela ahora: ");
		scanf("%s", ruta_aux);
	}
	escribirLog("Ruta archivo de configuración: %s\n", ruta_aux);
	t_config *config = config_create(ruta_aux);

	if (config == NULL)
	{
		free(config);
		free(ruta_aux);
		abort();
	}
	// Guardo los datos en una variable global
	infoConfig.puerto_prog = config_get_string_value(config, "PUERTO_PROG");
	infoConfig.puerto_cpu = config_get_string_value(config, "PUERTO_CPU");
	infoConfig.puerto_umc = config_get_string_value(config, "PUERTO_UMC");
	infoConfig.quantum = config_get_string_value(config, "QUANTUM");
	infoConfig.quantum_sleep = config_get_string_value(config, "QUANTUM_SLEEP");
	infoConfig.array_dispositivos = config_get_array_value(config, "IO_ID");
	infoConfig.array_io_sleeps = config_get_array_value(config, "IO_SLEEP");
	infoConfig.array_variables_compartidas = config_get_array_value(config, "SHARED_VARS");
	infoConfig.array_sem_id = config_get_array_value(config, "SEM_ID");
	infoConfig.array_sem_init = config_get_array_value(config, "SEM_INIT");
	infoConfig.stack_size = config_get_string_value(config, "STACK_SIZE");

	// No uso config_destroy(config) porque bugea
	free(config->path);
	free(config);
	free(ruta_aux);
}

void inicializarDirecciones(void)
{
	direccionParaConsola.sin_family = AF_INET;
	direccionParaConsola.sin_port = htons(atoi(infoConfig.puerto_prog));
	direccionParaConsola.sin_addr.s_addr = INADDR_ANY;
	memset(direccionParaConsola.sin_zero, '\0', sizeof(direccionParaConsola.sin_zero));

	direccionParaCPU.sin_family = AF_INET;
	direccionParaCPU.sin_port = htons(atoi(infoConfig.puerto_cpu));
	direccionParaCPU.sin_addr.s_addr = INADDR_ANY;
	memset(direccionParaCPU.sin_zero, '\0', sizeof(direccionParaCPU.sin_zero));

	direccionUMC.sin_family = AF_INET;
	direccionUMC.sin_port = htons(atoi(infoConfig.puerto_umc));
	direccionUMC.sin_addr.s_addr = INADDR_ANY;
	memset(direccionUMC.sin_zero, '\0', sizeof(direccionUMC.sin_zero));

}

void conectar_a_umc(void)
{
	fd_umc = socket(AF_INET, SOCK_STREAM, 0);
	if(connect(fd_umc, (struct sockaddr*) &direccionUMC, sizeof(struct sockaddr)) == -1)
	{
		perror("Error al conectar a la UMC");
		abort();
	}
}

int solicitarTamPaginas(void)
{
	t_mensajeHead head = {GET_TAM_PAGINA, 1, 1};
	t_mensaje mensaje;

	mensaje.head = head;
	mensaje.parametros = malloc(4);
	mensaje.mensaje_extra = malloc(1);

	int x = enviarMensaje(fd_umc, mensaje);

	freeMensaje(&mensaje);
	return x;
}

unsigned int mensaje_to_tamPag(t_mensaje *mensaje)
{
	unsigned int tam;
	memcpy((void*) &tam, (void*) (mensaje->parametros), sizeof(unsigned int));
	return tam;
}

void recibirTamPaginas(void)
{
	int x;
	if( (x = solicitarTamPaginas()) <= 0)
	{
		perror("Error al enviar solicitud de tamaño de paginas");
		abort();
	}

	int tamMsg;
	t_mensaje *mensaje = malloc(sizeof(t_mensaje));

	if((tamMsg = recibirMensaje(fd_umc, mensaje)) <= 0)
	{
		if(tamMsg == 0)
			printf("Se desconecto la umc\n");
		if(tamMsg < 0)
		{
			perror("Falla al recibir tamaño de paginas");
			abort();
		}
	}

	if(mensaje->head.codigo == RETURN_TAM_PAGINA)
	{
		tamPaginas = mensaje_to_tamPag(mensaje);
		escribirLog("Se recibio el tamaño de paginas: %d\n", tamPaginas);
	}
	else
	{
		perror("El mensaje recibido no corresponde a un aviso de tamaño de pagina\n");
		abort();
	}
	freeMensaje(mensaje);
	free(mensaje);
}

int enviarInfoUMC(unsigned int pid, unsigned int cantidadPaginas, const char *codigo)
{
	int ret;
	t_mensajeHead head = {INIT_PROG, 2, strlen(codigo) + 1};

	unsigned int *parametros = malloc(2 * sizeof(unsigned int));
	parametros[0] = pid;
	parametros[1] = cantidadPaginas;

	char *dupCodigo = strdup(codigo);

	t_mensaje mensaje = {head, parametros, dupCodigo};

	if((ret = enviarMensaje(fd_umc, mensaje)) == -1)
	{
		escribirLog("Error al enviar info a la umc\n");
		perror("Error al enviar info a la umc\n");
		abort();
	}

	freeMensaje(&mensaje);

	return ret;
}

void avisar_UMC_FIN_PROG(int pid)
{
	t_mensajeHead head = {FIN_PROG, 1, 0};
	t_mensaje mensaje;
	mensaje.head = head;
	mensaje.parametros[0] = pid;
	mensaje.mensaje_extra = NULL;

	if(enviarMensaje(fd_umc, mensaje) == -1)
	{
		escribirLog("Error al informar a la umc que termino el proceso %d\n", pid);
		abort();
	}
}

int seAlmacenoElProceso(void)
{
	t_mensaje *mensaje = malloc(sizeof(t_mensaje));

	if(recibirMensaje(fd_umc, mensaje) <= 0)
	{
		escribirLog("Error al recibir mensaje de la umc\n");
		perror("Error al recibir mensaje de la umc");
		abort();
	}

	if(mensaje->head.codigo == ALMACENAR_OK)
	{
		escribirLog("almacenar ok\n");
		freeMensaje(mensaje);
		free(mensaje);
		return 1;
	}
	if(mensaje->head.codigo == ALMACENAR_FAILED)
	{
		escribirLog("almacenar failed\n");
		freeMensaje(mensaje);
		free(mensaje);
		return 0;
	}

	perror("Mensaje desconocido");
	escribirLog("La funcion de verificar si se almaceno el proceso recibio un mensaje desconocido\n");
	freeMensaje(mensaje);
	free(mensaje);
	abort();
	return -1;//Aborta antes de retornar, pero si no pongo esto me tira warning :P
}

void avisar_Consola_ProgramaNoAlmacenado(int fd)
{
	//Avisar a la consola que no fue posible almacenar el proceso
	const char aviso[] = "Error: No ha sido posible almacenar el programa en memoria\nAbortado\n";
	t_mensajeHead head = {IMPRIMIR_TEXTO_PROGRAMA, 1, strlen(aviso) + 1};
	t_mensaje mensaje;
	mensaje.head = head;
	mensaje.parametros = malloc(4);
	mensaje.mensaje_extra = strdup(aviso);

	enviarMensaje(fd, mensaje);

	freeMensaje(&mensaje);

	//Avisar que aborte
	t_mensajeHead headExit = {EXIT_PROGRAMA, 1, 1};
	t_mensaje mensajeExit = {headExit, malloc(4), malloc(1)};

	enviarMensaje(fd, mensajeExit);

	freeMensaje(&mensajeExit);
}

void asociarPidConsola(int pid, int consola)
{
	escribirLog("Se intentara asociar el pid %d con al consola fd:%d\n", pid, consola);
	t_parPidConsola *par = malloc(sizeof(t_parPidConsola));
	par->pid = pid;
	par->fd_consola = consola;
	list_add(lista_Pares, par);
	escribirLog("Asociar ok\n");
}

void desasociarPidConsola(int pid)
{

	t_link_element *aux = lista_Pares->head, *aux2;

	if(aux == NULL)
	{
		escribirLog("Se intento desasociar un pid de una consola, estando la lista de pares vacía\n");
		return;
	}

	if(((t_parPidConsola*)(aux->data))->pid == pid)
	{
		lista_Pares->head = lista_Pares->head->next;
		lista_Pares->elements_count --;
		free(aux);
		escribirLog("Desasociar ok\n");
		return;
	}

	for(aux = lista_Pares->head; aux->next && ((t_parPidConsola*)(aux->next->data))->pid != pid ; aux = aux->next);
	if(aux->next)
	{
		aux2 = aux->next;
		aux->next = aux2->next;
		lista_Pares->elements_count --;
		free(aux2);
		escribirLog("Desasociar ok\n");
	}
	else
	{
		perror("desasociarPidConsola: No se encontro el pid a desasociar");
		escribirLog("desasociarPidConsola: No se encontro el pid a desasociar\n");
	}
}

void* mostrarParPorLog(void *par)
{
	escribirLog("Pid: %d, Consola: %d\n", ((t_parPidConsola*)par)->pid, ((t_parPidConsola*)par)->fd_consola);
	return NULL;
}

void mostrarParesPorLog(void)
{
	escribirLog("----------------------\n");
	escribirLog("Lista de pares actual:\n");
	list_map(lista_Pares, mostrarParPorLog);
	escribirLog("----------------------\n");
}

void mostrarListosPorLog(void)
{
	escribirLog("----------------------\n");
	escribirLog("Cola de Listos actual:\n");
	mostrarColaPorLog(cola_listos);
	escribirLog("----------------------\n");
}

int Consola_to_Pid(int consola)
{
	t_link_element *aux;
	for(aux = lista_Pares->head; aux; aux = aux->next)
		if( ((t_parPidConsola*)(aux->data))->fd_consola == consola )
			return ((t_parPidConsola*)(aux->data))->pid;

	return -1;
}

int Pid_to_Consola(int pid)
{
	t_link_element *aux;
		for(aux = lista_Pares->head; aux; aux = aux->next)
			if( ((t_parPidConsola*)(aux->data))->pid == pid )
				return ((t_parPidConsola*)(aux->data))->fd_consola;

		return -1;
}

int eliminarProcesoSegunPID(t_list *lista, int pid)
{
	t_link_element *aux = lista->head, *aux2;

	if(aux == NULL) return 0;

	if(((t_PCB*)(aux->data))->pid == pid)
	{
		lista->head = lista->head->next;
		lista->elements_count--;
		free(aux);
		return 1;
	}

	for(aux = lista->head, aux2 = lista->head->next; aux2; aux = aux->next, aux2 = aux2->next)
		if(((t_PCB*)(aux2->data))->pid == pid)
		{
			aux->next = aux2->next;
			lista->elements_count--;
			free(aux2);
			return 1;
		}
	return 0;
}

void abortarProceso(int pid)
{
	escribirLog("Se abortará el proceso %d\n", pid);

	if(FD_ISSET(pid, &conjunto_procesos_bloqueados))
	{
		int i;
		FD_CLR(pid, &conjunto_procesos_bloqueados);escribirLog("Se elimino del conj bloq\n");
		for(i = 0; i < cantidadDispositivos(); ++i)
			eliminarProcesoSegunPID(vector_dispositivos[i].cola->elements, pid);
	}
	if(FD_ISSET(pid, &conjunto_procesos_ejecutando))
	{
		FD_CLR(pid, &conjunto_procesos_ejecutando);
		//Hay que avisar a la cpu
	}
	if(FD_ISSET(pid, &conjunto_procesos_listos))
	{
		FD_CLR(pid, &conjunto_procesos_listos);
		eliminarProcesoSegunPID(cola_listos->elements, pid);
	}

	FD_SET(pid, &conjunto_procesos_abortados);escribirLog("Se seteo en el conj abort\n");

	actualizarMaster();escribirLog("Se actualizo el master\n");

	desasociarPidConsola(pid);

	mostrarParesPorLog();

	escribirLog("-----------------------\n");
	escribirLog("Se aborto el proceso %d\n", pid);
	escribirLog("-----------------------\n");
}

void imprimir(int imp, int consola)
{
	t_mensajeHead head = {IMPRIMIR_PROGRAMA, 1, 1};
	t_mensaje mensaje;
	mensaje.head = head;
	mensaje.parametros = malloc(4);
	mensaje.parametros[0] = imp;

	mensaje.mensaje_extra = malloc(1);

	if(enviarMensaje(consola, mensaje) == -1)
	{
		perror("Error al imprimir en consola\n");
		abort();
	}

	freeMensaje(&mensaje);
}

void imprimirTexto(const char *imp, int consola)
{
	t_mensajeHead head = {IMPRIMIR_TEXTO_PROGRAMA, 1, strlen(imp) + 1};
	t_mensaje mensaje;
	mensaje.head = head;
	mensaje.parametros = malloc(4);
	mensaje.mensaje_extra = strdup(imp);

	if( enviarMensaje(consola, mensaje) == -1 )
	{
		perror("Error al imprimir en consola\n");
		abort();
	}

	escribirLog("Se recibio el texto %s para imprimir\n", imp);

	freeMensaje(&mensaje);
}

void avisar_Consola_Fin_Programa(int consola)
{
	t_mensajeHead head = {EXIT_PROGRAMA, 1, 1};
	t_mensaje mensaje;
	mensaje.head = head;
	mensaje.parametros = malloc(4);
	mensaje.mensaje_extra = malloc(1);
	if(enviarMensaje(consola, mensaje) <= 0)
	{
		printf("Error al informar a la consola %d del fin del programa %d\n", consola, Consola_to_Pid(consola));
		abort();
	}

	escribirLog("Se informo a la consola(fd:[%d]) del fin del proceso %d\n", consola, Consola_to_Pid(consola));

	free(mensaje.parametros);
	free(mensaje.mensaje_extra);
}

void abrirPuertos(void)
{
	int yes1 = 1, yes2 = 1;/*para setsockopt()*/
	//Setear listener para consola
	if( (fd_listener_consola = socket(PF_INET, SOCK_STREAM, 0)) == -1 )
	{
		perror("Error Socket Listener Consola\n");
		exit(1);
	}

	if( setsockopt(fd_listener_consola, SOL_SOCKET, SO_REUSEADDR, &yes1, sizeof(int)) == -1 )
	{
		perror("Error setsockpt Listener Consola\n");
		exit(1);
	}
	if( bind(fd_listener_consola, (struct sockaddr*) &direccionParaConsola, sizeof(struct sockaddr)) == -1 )
	{
		perror("Error bind Listener Consola\n");
		exit(1);
	}
	if( listen(fd_listener_consola, BACKLOG) == -1 )
	{
		perror("Error listen(consola)\n");
		exit(1);
	}

	//Setear listener para cpu
	if( (fd_listener_cpu = socket(PF_INET, SOCK_STREAM, 0)) == -1 )
	{
		perror("Error Socket Escucha\n");
		exit(1);
	}

	if( setsockopt(fd_listener_cpu, SOL_SOCKET, SO_REUSEADDR, &yes2, sizeof(int)) == -1 )
	{
		perror("Error setsockpt()\n");
		exit(1);
	}
	if( bind(fd_listener_cpu, (struct sockaddr*) &direccionParaCPU, sizeof(struct sockaddr)) == -1 )
	{
		perror("Error setsockpt()\n");
		exit(1);
	}
	if( listen(fd_listener_cpu, BACKLOG) == -1 )
	{
		perror("Error listen(cpu)\n");
		exit(1);
	}
}

int maximofd(int fd1, int fd2)
{
	if(fd1 > fd2)
		return fd1;
	else
		return fd2;
}

int cantidadDispositivos(void)
{
	int i;
	for(i = 0; infoConfig.array_dispositivos[i]; ++i);
	return i;
}

int cantidadSleeps(void)
{
	int i;
	for(i = 0; infoConfig.array_io_sleeps[i]; ++i);
	return i;
}

void init_cantidad_varsComp(void)
{
	int i;
	for(i = 0; infoConfig.array_variables_compartidas[i]; ++i);
	cantidad_variables_compartidas = i;
}

void mostrarCompartidasPorLog(void)
{
	int i;
	escribirLog("Variables compartidas: [");
		for(i = 0; i < cantidad_variables_compartidas; ++i)
		{
			if(i < cantidad_variables_compartidas - 1)
				escribirLog("%s = %d, ", variables_compartidas[i].nombre, variables_compartidas[i].valor);
			else
				escribirLog("%s = %d]\n", variables_compartidas[i].nombre, variables_compartidas[i].valor);
		}
}

void init_cantidad_semaforos(void)
{
	int i;
	for(i = 0; infoConfig.array_sem_id[i]; ++i);
	cantidadSemaforos = i;
}

void mostrarSemaforosPorLog(void)
{
	int i;
	escribirLog("Semaforos: [");
	for(i = 0; i < cantidadSemaforos; ++i)
	{
		if(i < cantidadSemaforos - 1)
			escribirLog("%s = %d, ", semaforos[i].id, semaforos[i].cuenta);
		else
			escribirLog("%s = %d]\n", semaforos[i].id, semaforos[i].cuenta);
	}
}

char* agregarBang(const char *nombreSinBang)
{
	char *ret = malloc(strlen(nombreSinBang) + 2);
	ret[0] = '!';
	memcpy(ret+1, nombreSinBang, strlen(nombreSinBang));
	ret[strlen(nombreSinBang) + 1] = '\0';
	return ret;
}

void inicializarListas(void)
{
	FD_ZERO(&conjunto_procesos_listos);
	FD_ZERO(&conjunto_procesos_bloqueados);
	FD_ZERO(&conjunto_procesos_ejecutando);
	FD_ZERO(&conjunto_procesos_salida);
	FD_ZERO(&conjunto_cpus_libres);
	FD_ZERO(&conjunto_procesos_abortados);
	FD_ZERO(&conjunto_pids_abortados);
	lista_master_procesos = list_create();
	cola_bloqueados = queue_create();
	cola_listos = queue_create();
	lista_Pares = list_create();
	cola_cpus_disponibles = queue_create();

	stack_size = atoi(infoConfig.stack_size);

	int nDisp = cantidadDispositivos();
	if(nDisp != cantidadSleeps())
	{
		perror("Error Fatal: La cantidad de Dispositivos no coincide con la cantidad de IO_Sleeps");
		abort();
	}
	vector_dispositivos = malloc(nDisp * sizeof(t_dispositivo));
	int i;
	for(i = 0; i < nDisp; ++i)
	{
		vector_dispositivos[i].nombre = infoConfig.array_dispositivos[i];
		vector_dispositivos[i].io_sleep = atoi(infoConfig.array_io_sleeps[i]);
		vector_dispositivos[i].cola = queue_create();
		vector_dispositivos[i].atendiendo = NULL;
	}

	init_cantidad_varsComp();
	variables_compartidas = malloc(cantidad_variables_compartidas * sizeof(t_variable_compartida));
	for(i = 0; i < cantidad_variables_compartidas; ++i)
	{
		variables_compartidas[i].nombre = strdup(infoConfig.array_variables_compartidas[i]);
		variables_compartidas[i].valor = 0;
	}

	mostrarCompartidasPorLog();

	init_cantidad_semaforos();
	semaforos = malloc(cantidadSemaforos * sizeof(t_semaforo));
	for(i = 0; i < cantidadSemaforos; ++i)
	{
		semaforos[i].id = strdup(infoConfig.array_sem_id[i]);
		semaforos[i].cuenta = atoi(infoConfig.array_sem_init[i]);
		semaforos[i].cola = queue_create();
	}

	mostrarSemaforosPorLog();
}

void administrarConexiones(void)
{
	int maxfd/*cima del conjunto "master_fds" */, nbytes/*número de bytes recibidos del cliente*/;
	unsigned int tam = sizeof(struct sockaddr_in);//tamaño de datos pedido por accept()

	fd_set conj_master, conj_read;//conjuntos de fd's total(master) y para los que tienen datos para lectura(read)

	fd_set conj_consola, conj_cpu;

	FD_ZERO(&conj_master);
	FD_ZERO(&conj_read);
	FD_ZERO(&conj_consola);
	FD_ZERO(&conj_cpu);

	maxfd = maximofd(fd_listener_consola, maximofd(fd_listener_cpu, fd_umc));//Encontrar el maximo fd
	max_cpu = 0;//para que la primer cpu aceptada cumpla la condicion de ser mayor y se le asigne su valor
	max_proceso = 0;//se incrementará a medida que lleguen programas

	FD_SET(fd_listener_consola, &conj_master);
	FD_SET(fd_listener_cpu, &conj_master);
	FD_SET(fd_umc, &conj_master);

	while(1)
	{

		conj_read = conj_master;//se ponen a disposicion todos los fd para despues filtrar los que tengan entrada
		select(maxfd+1, &conj_read, NULL, NULL, NULL);

		for(fd_explorer = 0; fd_explorer <= maxfd; fd_explorer++)//recorrer todos los fd
		{
			if(FD_ISSET(fd_explorer, &conj_read))//hay datos!
			{
				if(fd_explorer == fd_listener_consola)//Hay una conexión de una nueva consola
				{
					if( (fd_new = accept(fd_listener_consola, (struct sockaddr*) &direccionCliente, &tam)) == -1)
					{
						escribirLog("Error Accept consola (fd[%d])", fd_explorer);
						FD_CLR(fd_new, &conj_master);
						close(fd_new);
					}

					else//Se acepto correctamente
					{
						FD_SET(fd_new, &conj_master);
						FD_SET(fd_new, &conj_consola);
						escribirLog("Se acaba de aceptar una conexion de una nueva Consola (fd[%d])\n", fd_new);
						if(fd_new > maxfd) maxfd = fd_new;
					}
				}
				if(fd_explorer == fd_listener_cpu)//Hay una conexión de una nueva cpu
				{
					if( (fd_new = accept(fd_listener_cpu, (struct sockaddr*) &direccionCliente, &tam)) == -1)
					{
						escribirLog("Error Accept cpu (fd[%d])", fd_explorer);
						FD_CLR(fd_new, &conj_master);
						close(fd_new);
					}
					else//se acepto correctamente
					{
						FD_SET(fd_new, &conj_master);
						FD_SET(fd_new, &conj_cpu);
						habilitarCPU(fd_new);
						escribirLog("Se acaba de aceptar una conexion de una nueva CPU (fd[%d])\n", fd_new);
						if(fd_new > max_cpu) max_cpu = fd_new;
						if(fd_new > maxfd) maxfd = fd_new;
					}
				}
				else//Hay datos entrantes
				{
					if(FD_ISSET(fd_explorer, &conj_consola))//Los datos vienen de una Consola
					{
						nbytes = recibirMensaje(fd_explorer, &mensajeConsola);//Recibir los datos en mensajeCOnsola

						if(nbytes <= 0)//Hubo un error o se desconecto
						{
							if(nbytes < 0)//Error
							{
								FD_CLR(fd_explorer, &conj_master);
								FD_CLR(fd_explorer, &conj_consola);
								close(fd_explorer);
								escribirLog("Error de recepción en una consola (fd[%d])\n", fd_explorer);
							}
							if(nbytes == 0)//Desconexion
							{
								FD_CLR(fd_explorer, &conj_master);
								FD_CLR(fd_explorer, &conj_consola);
								escribirLog("Se ha desconectado una consola (fd[%d])\n", fd_explorer);
								if(( FD_ISSET(Consola_to_Pid(fd_explorer), &conjunto_procesos_ejecutando) || FD_ISSET(Consola_to_Pid(fd_explorer), &conjunto_procesos_listos) || FD_ISSET(Consola_to_Pid(fd_explorer), &conjunto_procesos_bloqueados) ) )
								{
									avisar_UMC_FIN_PROG(Consola_to_Pid(fd_explorer));
									abortarProceso(Consola_to_Pid(fd_explorer));
								}
								else
								{
									desasociarPidConsola(Consola_to_Pid(fd_explorer));
								}

								close(fd_explorer);
							}
						}
						else//No hubo error ni desconexion
						{
							if(mensajeConsola.head.codigo == NUEVO_PROGRAMA)
							{
								escribirLog("Se ha recibido un programa de una Consola (fd[%d])\n", fd_explorer);

								//Crear PCB
								t_PCB *pcb = malloc(sizeof(t_PCB));
								escribirLog("-------------------\n");
								escribirLog("Tamaño recibido: %d\n", nbytes);
								escribirLog("-------------------\n");

								*pcb = crearPCB(mensajeConsola, ++max_proceso, tamPaginas, stack_size);

								escribirLog("--------------------------------\n");
								escribirLog("Se creo el PCB de pid:%d\n", pcb->pid);
								escribirLog("Cantidad de paginas que ocupa:%d\n", pcb->cantidadPaginas);
								escribirLog("--------------------------------\n");

								//Asociar el pcb a su consola
								asociarPidConsola(pcb->pid, fd_explorer);
								//Mostrar lista de pares por log
								mostrarParesPorLog();


								//Enviar a UMC: PID, Cant Paginas, codigo
								char *code = strdup(mensajeConsola.mensaje_extra);
								string_trim(&code);
								enviarInfoUMC(pcb->pid, pcb->cantidadPaginas, code);
								free(code);
								//Verificar que se pudo guardar el programa
								if(seAlmacenoElProceso())
								{
									escribirLog("Se almaceno correctamente el proceso %d\n", pcb->pid);

									//Agregar el PCB a Lista Master
									list_add(lista_master_procesos, pcb);

									t_PCB *new_pcb = malloc(sizeof(t_PCB));
									*new_pcb = mensaje_to_pcb(pcb_to_mensaje(*pcb, 0));

									//Agregar el PCB a Cola de Listos
									ponerListo(new_pcb);

									mostrarListosPorLog();
								}
								else
								{
									//freePCB(pcb); Revisar, da violacion de segmento
									escribirLog("La umc no pudo almacenar el proceso, se abortara el mismo\n");
									avisar_Consola_ProgramaNoAlmacenado(fd_explorer);
								}
								//	freeMensaje(&mensajeConsola);
							}

							if(mensajeConsola.head.codigo == ABORTAR_CONSOLA)
							{
								escribirLog("La consola planea abortarse, iniciar procedimiento de preparación para aborto\n");

								int elPID = Consola_to_Pid(fd_explorer);

								escribirLog("Se añade el pid %d al conjunto de procesos abortados\n", elPID);

								FD_SET(elPID, &conjunto_pids_abortados);
							}
						}

						continue;
					}

					if(FD_ISSET(fd_explorer, &conj_cpu))//Los datos vienen de una CPU
					{
						//recibirMensajeDeCPU()
						nbytes = recibirMensaje(fd_explorer, &mensajeCPU);//Recibir los datos al buffer


						if(nbytes <= 0)//Hubo un error o se desconecto
						{
							if(nbytes < 0)
							{
								FD_CLR(fd_explorer, &conj_master);
								FD_CLR(fd_explorer, &conj_cpu);
								close(fd_explorer);
								escribirLog("Error de recepción en una cpu (fd[%d])\n", fd_explorer);
								continue;
							}
							if(nbytes == 0)
							{
								FD_CLR(fd_explorer, &conj_master);
								FD_CLR(fd_explorer, &conj_cpu);
								deshabilitarCPU(fd_explorer);
								close(fd_explorer);
								escribirLog("Se ha desconectado una cpu (fd[%d])\n", fd_explorer);
							}
						}
						else//No hubo error ni desconexion
						{
							if(mensajeCPU.head.codigo == STRUCT_PCB)//Termino el quantum, poner en cola de listos
							{
								t_PCB *pcb = malloc(sizeof(t_PCB));
								*pcb = mensaje_to_pcb(mensajeCPU);

								escribirLog("----------------------------------------------------\n");
								escribirLog("Se recibio el proceso %d por finalización de quantum\n", pcb->pid);
								escribirLog("----------------------------------------------------\n");

								ponerListo(pcb);

								mostrarListosPorLog();
								FD_CLR(pcb->pid, &conjunto_procesos_ejecutando);
								actualizarMaster();
								//Poner a la cpu como libre
								habilitarCPU(fd_explorer);
								freeMensaje(&mensajeCPU);
								continue;
							}
							if(mensajeCPU.head.codigo == STRUCT_PCB_FIN)//Termino el programa
							{
								t_PCB *pcb = malloc(sizeof(t_PCB));
								*pcb = mensaje_to_pcb(mensajeCPU);

								int laConsola = Pid_to_Consola(pcb->pid);

								avisar_UMC_FIN_PROG(pcb->pid);
								FD_CLR(pcb->pid, &conjunto_procesos_ejecutando);

								terminar(pcb);

								avisar_Consola_Fin_Programa(laConsola);

								habilitarCPU(fd_explorer);
								freeMensaje(&mensajeCPU);
								continue;
							}

							if(mensajeCPU.head.codigo == STRUCT_PCB_FIN_ERROR)//Tuvo un error: Abortar
							{
								t_PCB *pcb = malloc(sizeof(t_PCB));
								*pcb = mensaje_to_pcb(mensajeCPU);

								int laConsola = Pid_to_Consola(pcb->pid);

								avisar_UMC_FIN_PROG(pcb->pid);
								FD_CLR(pcb->pid, &conjunto_procesos_ejecutando);

								terminar(pcb);

								imprimirTexto("El programa finalizo por un Error", laConsola);

								avisar_Consola_Fin_Programa(laConsola);

								habilitarCPU(fd_explorer);

								freeMensaje(&mensajeCPU);

								continue;
							}

							if(mensajeCPU.head.codigo == IMPRIMIR_NUM)//Quiere imprimir un numero
							{
								int pid = mensajeCPU.parametros[0];
								int imp = mensajeCPU.parametros[1];

								int laConsola = Pid_to_Consola(pid);
								escribirLog("Se recibio el numero %d para imprimir\n", imp);
								imprimir(imp, laConsola);
								freeMensaje(&mensajeCPU);
								continue;
							}
							if(mensajeCPU.head.codigo == IMPRIMIR_TEXTO)//Quiere imprimir texto
							{
								int pid = mensajeCPU.parametros[0];
								char* imp = strdup(mensajeCPU.mensaje_extra);
								imprimirTexto(imp, Pid_to_Consola(pid));
								free(imp);
								freeMensaje(&mensajeCPU);
								continue;
							}

							/**********************************/
							/*A partir de acá son System Calls*/
							/**********************************/



							if(mensajeCPU.head.codigo == OBTENER_COMPARTIDA)
							{
								escribirLog("----------------------------------------------------------\n");
								escribirLog("Se recibio una solicitud de lectura de variable compartida\n");

								int pid = mensajeCPU.parametros[0];
								char *nombreSinBang = strdup(mensajeCPU.mensaje_extra);
								char *nombreVariable = agregarBang(nombreSinBang);
								escribirLog("Nombre de la variable: %s\n", nombreVariable);

								t_mensajeHead head = {RETURN_OBTENER_COMPARTIDA, 2, 1};
								t_mensaje mensaje;
								mensaje.head = head;
								mensaje.parametros = malloc(2 * sizeof(int));

								int valor;
								if(existeVariable(nombreVariable))
								{
									valor = obtenerValorCompartida(nombreVariable);
									escribirLog("Valor obtenido: %d\n", valor);

									mensaje.parametros[0] = 1;//OK
									mensaje.parametros[1] = valor;

								}

								else
								{
									escribirLog("La variable compartida solicitada no existe, se advertira a la cpu para abortar el proceso %d\n", pid);

									mensaje.parametros[0] = 0;//FAIL
									mensaje.parametros[1] = 0;

									perror("Ver Log");
								}

								mensaje.mensaje_extra = malloc(1);
								mensaje.mensaje_extra[0] = '\0';


								if( enviarMensaje(fd_explorer, mensaje) == -1 )
								{
									perror("Error al retornar valor de variable compartida a una cpu");
									abort();
								}

								escribirLog("Se envio el valor a la cpu\n");
								escribirLog("----------------------------------------------------------\n");

								freeMensaje(&mensaje);
								free(nombreVariable);
								free(nombreSinBang);
								freeMensaje(&mensajeCPU);
								continue;
							}

							if(mensajeCPU.head.codigo == ASIGNAR_COMPARTIDA)
							{
								escribirLog("----------------------------------------------------------\n");
								escribirLog("Se recibio una solicitud de escritura para una variable compartida\n");
								char *nombreSinBang = strdup(mensajeCPU.mensaje_extra);
								char *nombre = agregarBang(nombreSinBang);
								escribirLog("La variable es: %s\n", nombre);
								int pid = mensajeCPU.parametros[0];
								int nuevoValor = mensajeCPU.parametros[1];
								escribirLog("Nuevo valor: %d\n", nuevoValor);

								t_mensajeHead head = {RETURN_ASIGNAR_COMPARTIDA, 2, 1};
								t_mensaje mensaje;
								mensaje.head = head;
								mensaje.parametros = malloc(2 * sizeof(int));

								if(existeVariable(nombre))
								{
									asignarCompartida(nombre, nuevoValor);
									mensaje.parametros[0] = 1;//OK
								}

								else
								{
									escribirLog("La variable compartida solicitada no existe, se advertira a la cpu para abortar el proceso %d\n", pid);

									mensaje.parametros[0] = 0;//FAIL

									perror("Ver Log");
								}

								mensaje.mensaje_extra = malloc(1);
								mensaje.mensaje_extra[0] = '\0';

								if( enviarMensaje(fd_explorer, mensaje) == -1 )
								{
									perror("Error al retornar valor de variable compartida a una cpu");
									abort();
								}

								escribirLog("Se envio el valor a la cpu\n");
								escribirLog("----------------------------------------------------------\n");

								mostrarCompartidasPorLog();
								freeMensaje(&mensaje);
								free(nombre);
								free(nombreSinBang);
								freeMensaje(&mensajeCPU);
								continue;
							}

							if(mensajeCPU.head.codigo == ENTRADA_SALIDA)//Pidio I/O: Bloquear y poner en la cola de espera que corresponda
							{
								//Sacarle los datos y luego volver a recibir mensaje
								int pid = mensajeCPU.parametros[0];
								int cantidadOperaciones = mensajeCPU.parametros[1];
								char* nombreDispositivo = strdup(mensajeCPU.mensaje_extra);

								escribirLog("Se recibio para I/O: ");
								escribirLog("pid: %d, ", pid);
								escribirLog("cantidad de operaciones a realizar: %d, ", cantidadOperaciones);
								escribirLog("nombre dispositivo: %s\n", nombreDispositivo);

								freeMensaje(&mensajeCPU);
								nbytes = recibirMensaje(fd_explorer, &mensajeCPU);
								if(nbytes <= 0)
								{
									perror("Error al recibir PCB que solicitó I/O");
									abort();
								}

								if(mensajeCPU.head.codigo == STRUCT_PCB_IO)//Verificar que sea un PCB
								{
									t_PCB *pcb = malloc(sizeof(t_PCB));
									*pcb = mensaje_to_pcb(mensajeCPU);

									bloquear(pcb, nombreDispositivo, cantidadOperaciones);
									actualizarMaster();
									habilitarCPU(fd_explorer);
									freeMensaje(&mensajeCPU);
									continue;
								}
								else
								{
									perror("Luego de una solicitud de I/O, no se recibio un PCB (Revisar codigos de header)");
									abort();
								}
								free(nombreDispositivo);
								freeMensaje(&mensajeCPU);
								continue;
							}

							if(mensajeCPU.head.codigo == WAIT)
							{
								//Sacarle los datos y luego volver a recibir mensaje
								int pid = mensajeCPU.parametros[0];
								char *nombre = strdup(mensajeCPU.mensaje_extra);

								escribirLog("Se recibio solicitud de Wait: ");
								escribirLog("pid: %d, semáforo: %s\n", pid, nombre);

								t_semaforo *s = nombre_to_semaforo(nombre);

								t_mensajeHead head = {CPU_WAIT, 1, 1};
								t_mensaje mensaje;
								mensaje.head = head;
								mensaje.parametros = malloc(sizeof(int));

								mensaje.mensaje_extra = malloc(1);
								mensaje.mensaje_extra[0] = '\0';

								if(s->cuenta <= 0)//Prediccion de que wait va a bloquear el proceso
								{
									mensaje.parametros[0] = 1;//Bloqueado

									if( enviarMensaje(fd_explorer, mensaje) == -1 )
									{
										perror("Wait: Error al enviar mensaje de bloqueo por sincronizacion a cpu");
										abort();
									}

									freeMensaje(&mensajeCPU);
									nbytes = recibirMensaje(fd_explorer, &mensajeCPU);
									if(nbytes <= 0)
									{
										perror("Error al recibir PCB que solicitó Wait()");
										abort();
									}

									if(mensajeCPU.head.codigo == STRUCT_PCB_WAIT)
									{
										t_PCB *pcb_wait = malloc(sizeof(t_PCB));
										*pcb_wait = mensaje_to_pcb(mensajeCPU);


										if(pid != pcb_wait->pid)
										{
											escribirLog("El pid de la señal de wait y el del pcb recibido no coinciden: %d /= %d --> abortar sistema\n", pid, pcb_wait->pid);
											perror("Abortado, ver Log");
											abort();
										}

										if(existeSemaforo(nombre))
										{
											escribirLog("El proceso %d ejecuto wait(%s)\n", pcb_wait->pid, nombre);
											semaforo_wait(pcb_wait, nombre_to_semaforo(nombre));
											mostrarSemaforosPorLog();
										}

										else
										{
											escribirLog("El proceso %d solicito wait a un semaforo que no existe: (%s), se abortara el proceso\n", pid, nombre);
											abortarProceso(pid);
										}

										habilitarCPU(fd_explorer);

									}

									else
									{
										escribirLog("Luego de una peticion de pcb por prediccion de bloqueo de wait(), se recibió un mensaje que no contiene un pcb (revisar códigos de mensaje): abortar sistema\n");
										perror("Abortado, ver Log");
										free(nombre);
										abort();
									}


								}

								else//Prediccion de que wait NO va a bloquear el proceso
								{
									mensaje.parametros[0] = 0;//Continuar

									if( enviarMensaje(fd_explorer, mensaje) == -1 )
									{
										perror("Wait: Error al enviar mensaje de no-bloqueo por sincronizacion a cpu");
										abort();
									}

									if(existeSemaforo(nombre))
									{
										escribirLog("El proceso %d ejecuto wait(%s)\n", pid, nombre);
										semaforo_wait(NULL, nombre_to_semaforo(nombre));
										mostrarSemaforosPorLog();
									}

									else
									{
										escribirLog("El proceso %d solicito wait a un semaforo que no existe: (%s), se abortara el proceso\n", pid, nombre);
										abortarProceso(pid);
									}

								}


								free(nombre);
								freeMensaje(&mensajeCPU);
								freeMensaje(&mensaje);
								continue;
							}

							if(mensajeCPU.head.codigo == SIGNAL)
							{
								int pid = mensajeCPU.parametros[0];
								char *nombre = strdup(mensajeCPU.mensaje_extra);
								if(existeSemaforo(nombre))
								{
									escribirLog("El proceso %d ejecutó signal(%s)\n", pid, nombre);
									semaforo_signal(nombre_to_semaforo(nombre));
									mostrarSemaforosPorLog();
								}

								else
								{
									escribirLog("El proceso %d solicito signal a un semaforo que no existe: (%s), se abortara el proceso\n", pid, nombre);
									abortarProceso(pid);
								}

								free(nombre);
								freeMensaje(&mensajeCPU);
								continue;
							}

							if(mensajeCPU.head.codigo == GET_ESTADO)
							{
								escribirLog("La cpu fd[%d] consulta por estado\n", fd_explorer);

								int pid = mensajeCPU.parametros[0];
								int retorno;

								if( FD_ISSET(pid, &conjunto_pids_abortados) )
								{
									escribirLog("Se avisará a la cpu que la consola planea abortar\n");
									retorno = 1;
								}
								else
								{
									escribirLog("EStado ok\n");
									retorno = 0;
								}

								t_mensajeHead headRetorno = {RETURN_GET_ESTADO, 1, 1};
								t_mensaje mensajeRetorno;

								mensajeRetorno.head = headRetorno;
								mensajeRetorno.parametros = malloc(sizeof(int));
								mensajeRetorno.parametros[0] = retorno;
								mensajeRetorno.mensaje_extra = malloc(1);
								mensajeRetorno.mensaje_extra[0] = '\0';

								if( enviarMensaje(fd_explorer, mensajeRetorno) <= 0 )
								{
									perror("Error al avisar a la cpu sobre el estado de la consola\n");
									abort();
								}

								free(mensajeRetorno.parametros);
								free(mensajeRetorno.mensaje_extra);
							}

							if(mensajeCPU.head.codigo == STRUCT_PCB_ABORT_CONSOLA)
							{
								escribirLog("Se recibio el pcb de la consola que abortará\n");

								t_PCB pcb = mensaje_to_pcb(mensajeCPU);

								t_mensajeHead headParaConsola = {RETURN_ABORTAR_CONSOLA, 1, 1};
								t_mensaje mensajeParaConsola;
								mensajeParaConsola.head = headParaConsola;
								mensajeParaConsola.parametros = malloc(sizeof(int));
								mensajeParaConsola.parametros[0] = 1;//basura
								mensajeParaConsola.mensaje_extra = malloc(1);
								mensajeParaConsola.mensaje_extra[0] = '\0';

								if( enviarMensaje(Pid_to_Consola(pcb.pid), mensajeParaConsola) <= 0 )
								{
									perror("Error al avisar a la consola que puede abortar\n");
									abort();
								}

								habilitarCPU(fd_explorer);
								escribirLog("Se habilitó nuevamente la cpu fd[%d]", fd_explorer);
							}

							freeMensaje(&mensajeCPU);

						}

					}

				}
			}
		}
	}
}


void* llamar_RoundRobin(void *data)
{
	unsigned short int quantum = (unsigned short int) atoi(infoConfig.quantum);
	unsigned int quantumSleep = (unsigned int) atoi(infoConfig.quantum_sleep);
	roundRobin(quantum, quantumSleep, cola_listos, cola_bloqueados, NULL);
	return NULL;
}

void* llamar_AdministrarConexiones(void *data)
{
	administrarConexiones();
	return NULL;
}

void* llamar_Interfaz(void* data)
{
	interfaz();
	return NULL;
}

void montarHilos(void)
{
	pthread_t hilo_RoundRobin, hilo_AdministrarConexiones, hilo_Interfaz;

	pthread_create(&hilo_AdministrarConexiones, NULL, llamar_AdministrarConexiones, NULL);
	pthread_create(&hilo_RoundRobin, NULL, llamar_RoundRobin, NULL);
	pthread_create(&hilo_Interfaz, NULL, llamar_Interfaz, NULL);

	pthread_join(hilo_AdministrarConexiones, NULL);
	pthread_join(hilo_RoundRobin, NULL);
	pthread_join(hilo_Interfaz, NULL);
}

void estado_to_string(int estado, char *string)
{
	switch(estado)
	{
	case 0:
		strcpy(string, "Nuevo");
		break;
	case 1:
		strcpy(string, "Listo");
		break;
	case 2:
		strcpy(string, "Ejecutando");
		break;
	case 3:
		strcpy(string, "Bloqueado");
		break;
	case 4:
		strcpy(string, "Terminado");
		break;
	default:
		perror("Se recibio un PCB con estado de proceso invalido");
	}
}
