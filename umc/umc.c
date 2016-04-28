#include "umc.h"


void leerArchivoConfig()
{

	t_config *config = config_create("config.conf");

	if (config == NULL) {
		free(config);
		abort();
	}
	// Guardo los datos en una variable global
	infoConfig.ip = config_get_string_value(config, "IP");
	infoConfig.puerto = config_get_string_value(config, "PUERTO");

	// No uso config_destroy(config) porque bugea
	free(config->path);
	free(config);
	return;
}

struct sockaddr_in setDireccion()
{
	struct sockaddr_in direccion;
	direccion.sin_family = AF_INET;
	direccion.sin_addr.s_addr = inet_addr(infoConfig.ip);
	direccion.sin_port = htons (atoi(infoConfig.puerto));
	memset(&(direccion.sin_zero), '\0', 8);

	return direccion;
}


int buscarConexiones()
{
	direccionServidorUMC = setDireccion();

	servidorUMC = socket(AF_INET, SOCK_STREAM, 0);

	int activado = 1;
	setsockopt(servidorUMC, SOL_SOCKET, SO_REUSEADDR, &activado, sizeof(activado)); //para reutilizar dirreciones

	if (bind(servidorUMC, (void*) &direccionServidorUMC, sizeof(direccionServidorUMC)) != 0) {
		perror("Falló asociando el puerto");
		return 0;
	}

	printf("Estoy escuchando\n");
	listen(servidorUMC, 100);
	return 1;
}

void aceptarConexion() //Por ahora del CPU
{
	struct sockaddr_in direccionCPU;
	unsigned int tamanioDireccion;
	clienteCPU = accept(servidorUMC, (void*) &direccionCPU, &tamanioDireccion);
	return ;
}

int recibirDatos()
{
	char* buffer = malloc(100);
	int bytesRecibidos = recv(clienteCPU, buffer, 100, 0);
	if (bytesRecibidos <= 0) {
		perror("El CPU se desconecto");
		return 0;
	}
	printf("El mensaje recibido es: %s\n", buffer);
	free(buffer);
	return 1;
}


