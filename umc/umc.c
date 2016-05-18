#include "umc.h"


void leerArchivoConfig()
{

	t_config *config = config_create("config.conf");

	if (config == NULL) {
		free(config);
		abort();
	}

	infoConfig.ip = config_get_string_value(config, "IP");
	infoConfig.puertoUMC = config_get_string_value(config, "PUERTO");
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

void gestionarConexiones() // el select basicamente
{
	fd_set fdsParaLectura;
	int maximoFD;
	int fdBuscador;

	FD_ZERO(&master);
	FD_ZERO(&fdsParaLectura);

	maximoFD = recibirConexiones();

	while(1){
		fdsParaLectura = master;

		if (  select(maximoFD+1,&fdsParaLectura,NULL,NULL,NULL) == -1  ){ //comprobar si el select funciona
			perror("Error en el select");
			abort();
		}

		for(fdBuscador=0; fdBuscador <= maximoFD; fdBuscador++) // explora los FDs que estan listos para leer
		{
			if( FD_ISSET(fdBuscador,&fdsParaLectura) ) { //entra una conexion, la acepta y la agrega al master
				if(fdBuscador == servidorUMC)
					aceptarConexion();
				else{
					FD_SET(clienteUMC, &master);
					if(clienteUMC > maximoFD) //Actualzar el maximo fd
						maximoFD = clienteUMC;
				}

			}
			else{
				 if(recibirDatos() > 0)
					 enviarDatos(); // al swap para probar
			}
		}
	}

	return;
}
int recibirConexiones()
{

	direccionServidorUMC = setDireccion(infoConfig.puertoUMC);

	servidorUMC = socket(AF_INET, SOCK_STREAM, 0);

	int activado = 1;
	setsockopt(servidorUMC, SOL_SOCKET, SO_REUSEADDR, &activado, sizeof(activado)); //para reutilizar dirreciones

	if (bind(servidorUMC, (void*) &direccionServidorUMC, sizeof(direccionServidorUMC)) != 0) {
		perror("Falló asociando el puerto\n");
		abort();
	}

	printf("Estoy escuchando\n");
	listen(servidorUMC, 100);
	FD_SET(servidorUMC,&master);

	return servidorUMC;
}

void aceptarConexion()
{
	struct sockaddr_in direccion;  //del cpu o del nucleo
	unsigned int tamanioDireccion;
	clienteUMC = accept(servidorUMC, (void*) &direccion, &tamanioDireccion);
	return ;
}

int recibirDatos()
{
	buffer = malloc(100);
	int bytesRecibidos = recv(clienteUMC, buffer, 100, 0);
	if (bytesRecibidos <= 0) {
		perror("El cliente se desconecto\n");
		close(clienteUMC);
		FD_CLR(clienteUMC, &master);
		return 0;
	}
	printf("UMC: El mensaje recibido es: %s\n", buffer);
	return bytesRecibidos;
}

void conectarAlSWAP()
{
	direccionServidorSWAP = setDireccion(infoConfig.puertoSWAP);
	clienteSWAP = socket(AF_INET, SOCK_STREAM, 0);
		if (connect(clienteSWAP, (void*) &direccionServidorSWAP, sizeof(direccionServidorSWAP)) != 0) {
			perror("No se pudo conectar");
			abort();
		}
		return ;
}
void enviarDatos() // Por ahora al swap
{
	send(clienteSWAP, buffer, strlen(buffer), 0);
	free(buffer);
	return;
}

int main(){
	//Config
	leerArchivoConfig();

	conectarAlSWAP();

	//servidor
	gestionarConexiones();
	//recibirConexiones();
	//aceptarConexion();
	//recibirDatos();

	//cliente
	//enviarDatos();

	return 0;
}


