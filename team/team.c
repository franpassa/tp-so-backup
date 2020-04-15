#include "team.h"

int main(){

	int socket = 0;

	//Inicializo el log
	t_log* logger = iniciar_logger();

	log_info(logger, "Log del proceso 'team' creado.");

	//Pruebo archivo de configuracion
	t_config* config = config_create(PATH_CONFIG);

	char* nombre = config_get_string_value(config, "NOMBRE");

	printf("\nEl nombre es: %s", nombre);

	//Finalizo el programa
	terminar_programa(socket, logger, config);

	return 0;
}
