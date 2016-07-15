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

	free(mensaje.parametros);
	free(mensaje.mensaje_extra);
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
}

int enviarInfoUMC(unsigned int pid, unsigned int cantidadPaginas, const char *codigo)
{
	int ret;
	t_mensajeHead head = {INIT_PROG, 2, strlen(codigo)};

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

	free(parametros);
	free(dupCodigo);

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
		return 1;
	}
	if(mensaje->head.codigo == ALMACENAR_FAILED)
	{
		escribirLog("almacenar failed\n");
		freeMensaje(mensaje);
		return 0;
	}

	perror("Mensaje desconocido");
	escribirLog("La funcion de verificar si se almaceno el proceso recibio un mensaje desconocido\n");
	freeMensaje(mensaje);
	abort();
	return -1;//Aborta antes de retornar, pero si no pongo esto me tira warning :P
}

void avisar_Consola_ProgramaNoAlmacenado(int fd)
{
	const char aviso[] = "Error: No ha sido posible almacenar el programa en memoria\nAbortado\n";
	t_mensajeHead head = {IMPRIMIR_TEXTO_PROGRAMA, 0, strlen(aviso)};
	t_mensaje mensaje;
	mensaje.head = head;
	mensaje.mensaje_extra = strdup(aviso);
	enviarMensaje(fd, mensaje);
	free(mensaje.mensaje_extra);
}

void asociarPidConsola(int pid, int consola)
{
	t_parPidConsola *par = malloc(sizeof(t_parPidConsola));
	par->pid = pid;
	par->fd_consola = consola;
	list_add(lista_Pares, par);
}

void desasociarPidConsola(int pid)
{
	t_link_element *aux = lista_Pares->head, *aux2;

	if(((t_parPidConsola*)(aux->data))->pid == pid)
	{
		lista_Pares->head = lista_Pares->head->next;
		free(aux);
		return;
	}

	for(aux = lista_Pares->head; aux->next && ((t_parPidConsola*)(aux->next->data))->pid != pid ; aux = aux->next);
	if(aux->next)
	{
		aux2 = aux->next;
		aux->next = aux2->next;
		free(aux2);
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
	if(FD_ISSET(pid, &conjunto_procesos_bloqueados))
	{
		FD_CLR(pid, &conjunto_procesos_bloqueados);
		eliminarProcesoSegunPID(cola_bloqueados->elements, pid);
	}
	if(FD_ISSET(pid, &conjunto_procesos_ejecutando))
	{
		FD_CLR(pid, &conjunto_procesos_ejecutando);
	}
	if(FD_ISSET(pid, &conjunto_procesos_listos))
	{
		FD_CLR(pid, &conjunto_procesos_listos);
		eliminarProcesoSegunPID(cola_listos->elements, pid);
	}

	FD_SET(pid, &conjunto_procesos_abortados);

	actualizarMaster();

	desasociarPidConsola(pid);

	mostrarParesPorLog();

	escribirLog("-----------------------\n");
	escribirLog("Se aborto el proceso %d\n", pid);
	escribirLog("-----------------------\n");
}

void imprimir(int imp, int consola)
{
	t_mensajeHead head = {IMPRIMIR_PROGRAMA, 1, 0};
	t_mensaje mensaje;
	mensaje.head = head;
	mensaje.parametros[0] = imp;

	if(enviarMensaje(consola, mensaje) == -1)
	{
		perror("Error al imprimir en consola\n");
		abort();
	}
}

void imprimirTexto(const char *imp, int consola)
{
	t_mensajeHead head = {IMPRIMIR_TEXTO_PROGRAMA, 1, 0};
	t_mensaje mensaje;
	mensaje.head = head;
	mensaje.mensaje_extra = strdup(imp);

	if( enviarMensaje(consola, mensaje) == -1 )
	{
		perror("Error al imprimir en consola\n");
		abort();
	}
}

void avisar_Consola_Fin_Programa(int consola)
{
	t_mensajeHead head = {EXIT_PROGRAMA, 1, 0};
	t_mensaje mensaje;
	mensaje.head = head;
	enviarMensaje(consola, mensaje);
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

void inicializarListas(void)
{
	FD_ZERO(&conjunto_procesos_listos);
	FD_ZERO(&conjunto_procesos_bloqueados);
	FD_ZERO(&conjunto_procesos_ejecutando);
	FD_ZERO(&conjunto_procesos_salida);
	lista_master_procesos = list_create();
	cola_bloqueados = queue_create();
	cola_listos = queue_create();
	lista_Pares = list_create();
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
						FD_SET(fd_new, &conjunto_cpus_libres);
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
								abortarProceso(Consola_to_Pid(fd_explorer));
								close(fd_explorer);
							}
						}
						else//No hubo error ni desconexion
						{
							escribirLog("Se ha recibido un programa de una Consola (fd[%d])\n", fd_explorer);

							//Crear PCB
							t_PCB *pcb = malloc(sizeof(t_PCB));
							escribirLog("-------------------\n");
							escribirLog("Tamaño recibido: %d\n", nbytes);
							escribirLog("-------------------\n");

							*pcb = crearPCB(mensajeConsola, ++max_proceso, tamPaginas);

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

							//Verificar que se pudo guardar el programa
							if(seAlmacenoElProceso())
							{
								escribirLog("Se almaceno correctamente el proceso %d\n", pcb->pid);

								//Agregar el PCB a Lista Master
								list_add(lista_master_procesos, pcb);

								//Agregar el PCB a Cola de Listos
								ponerListo(pcb);

								mostrarListosPorLog();
							}
							else
							{
								free(pcb->indiceEtiquetas);
								free(pcb->indiceCodigo);
								free(pcb->indiceStack);
								free(pcb);
								escribirLog("No se almaceno el proceso, se abortara el mismo\n");
								avisar_Consola_ProgramaNoAlmacenado(fd_explorer);
							}

						}
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
								FD_CLR(fd_explorer, &conjunto_cpus_libres);
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
								FD_SET(fd_explorer, &conjunto_cpus_libres);
								continue;
							}

							if(mensajeCPU.head.codigo == STRUCT_PCB_FIN)//Termino el programa
							{
								t_PCB *pcb = malloc(sizeof(t_PCB));
								*pcb = mensaje_to_pcb(mensajeCPU);
								avisar_UMC_FIN_PROG(pcb->pid);
								FD_CLR(pcb->pid, &conjunto_procesos_ejecutando);
								desasociarPidConsola(pcb->pid);
								mostrarParesPorLog();
								terminar(pcb);
								avisar_Consola_Fin_Programa(Pid_to_Consola(pcb->pid));
								actualizarMaster();
								FD_SET(fd_explorer, &conjunto_cpus_libres);
								continue;
							}

							if(mensajeCPU.head.codigo == STRUCT_PCB_FIN_ERROR)//Tuvo un error: Abortar
							{
								t_PCB *pcb = malloc(sizeof(t_PCB));
								*pcb = mensaje_to_pcb(mensajeCPU);
								avisar_UMC_FIN_PROG(pcb->pid);
								imprimirTexto("Programa Abortado", Pid_to_Consola(pcb->pid));
								avisar_Consola_Fin_Programa(Pid_to_Consola(pcb->pid));
								abortarProceso(pcb->pid);
								actualizarMaster();
								FD_SET(fd_explorer, &conjunto_cpus_libres);
								continue;
							}

							if(mensajeCPU.head.codigo == IMPRIMIR_NUM)
							{
								int pid = mensajeCPU.parametros[0];
								int imp = mensajeCPU.parametros[1];
								imprimir(imp, Pid_to_Consola(pid));
								continue;
							}
							if(mensajeCPU.head.codigo == IMPRIMIR_TEXTO)
							{
								int pid = mensajeCPU.parametros[0];
								char* imp = strdup(mensajeCPU.mensaje_extra);
								imprimirTexto(imp, Pid_to_Consola(pid));
								free(imp);
								continue;
							}

							/**********************************/
							/*A partir de acá son System Calls*/
							/**********************************/



							if(mensajeCPU.head.codigo == OBTENER_COMPARTIDA)
							{
								continue;
							}

							if(mensajeCPU.head.codigo == ASIGNAR_COMPARTIDA)
							{
								continue;
							}

							if(mensajeCPU.head.codigo == ENTRADA_SALIDA)//Pidio I/O, Bloquear
							{
								//Sacarle los datos y luego volver a recibir mensaje
								int pid = mensajeCPU.parametros[0];(void) pid;
								int tiempo = mensajeCPU.parametros[1];(void)tiempo;
								char* nombreDispositivo = strdup(mensajeCPU.mensaje_extra);(void)nombreDispositivo;

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
									FD_CLR(pcb->pid, &conjunto_procesos_ejecutando);
									bloquear(pcb);
									actualizarMaster();
									FD_SET(fd_explorer, &conjunto_cpus_libres);
									continue;
								}
								else
								{
									perror("Luego de una solicitud de I/O, no se recibio un PCB (Revisar codigos de header)");
									abort();
								}

								continue;
							}

							if(mensajeCPU.head.codigo == WAIT)
							{
								if(mensajeCPU.head.codigo == STRUCT_PCB_WAIT)
								{
									continue;
								}

								continue;
							}

							if(mensajeCPU.head.codigo == SIGNAL)
							{
								continue;
							}

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
	unsigned int quantumSleep = (unsigned short int) atoi(infoConfig.quantum_sleep);
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
