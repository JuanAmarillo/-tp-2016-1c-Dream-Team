#include "nucleo.h"

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
/*
 * inicializarMiDireccion();
 * Parametros: -
 * Descripcion: Procedimiento que inicializa la estructura de dirección del propio núcleo
 * Return: -
 */
void inicializarMiDireccion(void)
{
	miDireccion.sin_family = AF_INET;
	miDireccion.sin_port = htons(MIPUERTO);
	miDireccion.sin_addr.s_addr = INADDR_ANY;
	memset(miDireccion.sin_zero, '\0', sizeof(miDireccion.sin_zero));
}
