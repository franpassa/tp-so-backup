#include "headers/team.h"

int main(){

	inicializarPrograma(); //Inicializo logger y config

	char** posicionesEntrenadores = config_get_array_value(config,"POSICIONES_ENTRENADORES");

	uint32_t cant = cantidadTotalEntrenadores(posicionesEntrenadores);

	printf("La cantidad total de entrenadores es: %d", cant);

	liberarPosicionesEntrenadores(posicionesEntrenadores);

	terminar_programa(); //Finalizo el programa

	return 0;
}

t_log* iniciar_logger(void) {
	return log_create(PATH_LOG, PROGRAM_NAME, 1, LOG_LEVEL_INFO);
}

t_config* leer_config(void) {
	return config_create(PATH_CONFIG);
}

void terminar_programa(void){
	log_destroy(logger);
	config_destroy(config);
}

void inicializarPrograma(void){
	//Inicializo el log
	logger = iniciar_logger();

	log_info(logger, "Log del proceso 'team' creado.\n");

	//Leo el archivo de configuracion
	config = config_create(PATH_CONFIG);

	printf("\n\nArchivo de configuracion leido.\n\n");
}
