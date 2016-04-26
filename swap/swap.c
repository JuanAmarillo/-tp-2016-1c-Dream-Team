/*
 * swap.c
 *
 *  Created on: 26/4/2016
 *      Author: utnso
 */
#include <commons/config.h>
#include <commons/string.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include "swap.h"

int main(){
	setUMCAdress();
	return 0;
}

void setUMCAdress(){
	t_config *config = config_create("config.conf");

		if (config == NULL) {
			free(config);
			abort();
		}
	dirUMC.ip_umc = config_get_string_value(config, "IP_UMC");
	dirUMC.puerto_umc = config_get_string_value(config, "IP_UMC");

	return;
}
