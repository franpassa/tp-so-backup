/*
 * main.c
 *
 *  Created on: 28 feb. 2019
 *      Author: utnso
 */

#include "tp0.h"

int main(void)
{
	/*---------------------------------------------------PARTE 2-------------------------------------------------------------*/
	char* ip;
	char* puerto;

	t_log* logger;
	t_config* config;

	logger = iniciar_logger();

	log_info(logger, "Soy un log");

	config = leer_config();


	/*---------------------------------------------------PARTE 3-------------------------------------------------------------*/

	//antes de continuar, tenemos que asegurarnos que el servidor esté corriendo porque lo necesitaremos para lo que sigue.

	//crear conexion

	ip = config_get_string_value(config, "IP");
	puerto = config_get_string_value(config, "PUERTO");

	int socket = crear_conexion(ip, puerto);
	log_info(logger, "Conexión creada - Socket %d", socket);

	//enviar mensaje

	enviar_mensaje("hola 123", socket);

	//recibir mensaje

	char* mensaje = recibir_mensaje(socket);

	//loguear mensaje recibido

	log_info(logger, "%s", mensaje);

	free(mensaje);

	terminar_programa(socket, logger, config);
}

t_log* iniciar_logger(void) {
	return log_create(PATH_LOG, PROGRAM_NAME, 1, LOG_LEVEL_INFO);
}

t_config* leer_config(void) {
	return config_create(PATH_CONFIG);
}

void terminar_programa(int conexion, t_log* logger, t_config* config) {

	close(conexion);
	log_destroy(logger);
	config_destroy(config);

}
