#include "umc.h"


void leerArchivoConfig()
{

	t_config *config = config_create("config.conf");

	if (config == NULL) {
		free(config);
		abort();
	}

	infoConfig.ip = config_get_string_value(config, "IP");
	infoConfig.puertoUMC = config_get_string_value(config, "PUERTO_UMC");
	infoConfig.puertoSWAP = config_get_string_value(config, "PUERTO_SWAP");

	free(config->path);
	free(config);
	return;
}

struct sockaddr_in setDireccion(const char *puerto)
{
	struct sockaddr_in direccion;
	direccion.sin_family = AF_INET;
	direccion.sin_addr.s_addr = inet_addr(infoConfig.ip);
	direccion.sin_port = htons (atoi(puerto));
	memset(&(direccion.sin_zero), '\0', 8);

	return direccion;
}


int recibirConexiones() // por ahora del cpu
{
	direccionServidorUMC = setDireccion(infoConfig.puertoUMC);

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
	buffer = malloc(100);
	int bytesRecibidos = recv(clienteCPU, buffer, 100, 0);
	if (bytesRecibidos <= 0) {
		perror("El CPU se desconecto");
		return 0;
	}
	printf("El mensaje recibido es: %s\n", buffer);
	return 1;
}

int conectarAlSWAP()
{
	direccionServidorSWAP = setDireccion(infoConfig.puertoSWAP);
	clienteSWAP = socket(AF_INET, SOCK_STREAM, 0);
		if (connect(clienteSWAP, (void*) &direccionServidorSWAP, sizeof(direccionServidorSWAP)) != 0) {
			perror("No se pudo conectar");
			return 0;
		}
		return 1;
}
void enviarDatos() // Por ahora al swap
{
	send(clienteSWAP, buffer, strlen(buffer), 0);
	free(buffer);
	return;
}

int main(){ // es feo pero es para probar
	if(recibirConexiones() != 0)
		aceptarConexion();
	else return -1;

	if(recibirDatos() != 0)
		if(conectarAlSWAP() != 0)
			enviarDatos();
	else return -2; else return -3;

	return 0;
}


