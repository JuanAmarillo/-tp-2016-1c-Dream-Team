#include "nucleo.h"

int main(void)
{
	// Leer archivo config.conf
	leerArchivoConfig();
	
	//file descriptor para escuchar (listener), para una nueva conexión (new) y para explorar conexiones (explorer)
	int fd_listener, fd_new, fd_explorer;
	
	fd_set master_fds, read_fds;//conjuntos de fd's total(master) y para los que tienen datos para lectura(read)
	struct direccionCliente;//direccion del cliente
	
	int maxfd/*cima del conjunto "master_fds" */, yes = 1/*para setsockopt()*/, nbytes/*número de bytes recibidos del cliente*/;
	char buffer[100];//buffer para datos recibidos del cliente
	unsigned int tam = sizeof(struct sockaddr_in);//tamaño de datos pedido por accept()

	FD_ZERO(&master_fds);
	FD_ZERO(&read_fds);
	
	if( fd_listener = socket(PF_INET, SOCK_STREAM, 0) == -1 )
	{
		perror("Error Socket Listener\n");
		exit(1);
	}
	
	if( setsockopt(fd_listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1 )
	{
		perror("Error setsockpt()\n");
		exit(1);
	}
	if( bind(fd_listener, (struct sockaddr*) &miDireccion, sizeof(struct sockaddr)) == -1 )
	{
		perror("Error setsockpt()\n");
		exit(1);	
	}
	listen(fd_listener, BACKLOG);

	maxfd = fd_listener;//Por ahora el max es el listener
	FD_SET(fd_listener, &master_fds);
	
	return EXIT_SUCCESS;
}

/*
 * leerArchivoConfig();
 * Parametros: -
 * Descripcion: Procedimiento que lee el archivo config.conf y lo carga en la variable infoConfig
 * Return: -
 */
void leerArchivoConfig() {

	t_config *config = config_create("config.conf");

	if (config == NULL) {
		free(config);
		abort();
	}
	// Guardo los datos en una variable global
	infoConfig.puerto_prog = config_get_string_value(config, "PUERTO_PROG");
	infoConfig.puerto_cpu = config_get_string_value(config, "PUERTO_CPU");
	infoConfig.puerto_umc = config_get_string_value(config, "PUERTO_UMC");

	// No uso config_destroy(config) porque bugea
	free(config->path);
	free(config);
}
