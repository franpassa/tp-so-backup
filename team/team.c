#include "headers/team.h"

	char* get_posicion(char* array,int pos){
		return string_substring(array,pos*8+2,6);
	}

int main(){

	int socket = 0;

	//Inicializo el log
	t_log* logger = iniciar_logger();

	log_info(logger, "Log del proceso 'team' creado.\n");

	//Leo el archivo de configuracion
	t_config* config = config_create(PATH_CONFIG);
	printf("\n\nArchivo de configuracion leido.\n\n");

	char* string = config_get_string_value(config,"POSICIONES_ENTRENADORES");

	char* string2 = get_posicion(string,0);

	printf("el string es: %s\n\n", string2);

	char** array = string_get_string_as_array(string2);

	printf("La posicion 0 del array es: %d\n", atoi(array[0]));

	//Finalizo el programa
	terminar_programa(socket, logger, config);

	return 0;
}

t_log* iniciar_logger(void) {
	return log_create(PATH_LOG, PROGRAM_NAME, 1, LOG_LEVEL_INFO);
}

t_config* leer_config(void) {
	return config_create(PATH_CONFIG);
}

void terminar_programa(int conexion, t_log* logger, t_config* config){
	close(conexion);
	log_destroy(logger);
	config_destroy(config);

}
