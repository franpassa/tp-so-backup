#include "headers/team.h"

	int posicionXEntrenador(int nroEntrenador, char** posicionesEntrenadores){
		char* posTemp1 = malloc(sizeof(posicionesEntrenadores[0]));
		posTemp1 = posicionesEntrenadores[nroEntrenador];
		char** posTemp2 = string_split(posTemp1,"|");
		int posX = atoi(posTemp2[0]);
		return posX;
	}

	int posicionYEntrenador(int nroEntrenador, char** posicionesEntrenadores){
		char* posTemp1 = posicionesEntrenadores[nroEntrenador];
		char** posTemp2 = string_split(posTemp1,"|");
		int posY = atoi(posTemp2[1]);
		return posY;
	}

	void liberarEntrenadores(char** posicionesEntrenadores){
		for(int i=0;i<(sizeof(posicionesEntrenadores)/sizeof(posicionesEntrenadores[0]));i++){
			free(posicionesEntrenadores[i]);
		}
		free(posicionesEntrenadores);
	}

	struct Entrenador{
		int posX;
		int posY;
		t_list* pokesAtrapados;
		t_list* pokesObjetivos;
		int idEntrenador;

	};

int main(){

	int socket = 0;

	//Inicializo el log
	t_log* logger = iniciar_logger();

	log_info(logger, "Log del proceso 'team' creado.\n");

	//Leo el archivo de configuracion
	t_config* config = config_create(PATH_CONFIG);
	printf("\n\nArchivo de configuracion leido.\n\n");

	char** posicionesEntrenadores = config_get_array_value(config,"POSICIONES_ENTRENADORES");

//	int posicion0 = posicionXEntrenador(0,posicionesEntrenadores);
//
//	int posicion1 = posicionYEntrenador(0,posicionesEntrenadores);
//
//	printf("%d", posicion0);
//
//	printf(", %d", posicion1);
//
//	liberarEntrenadores(posicionesEntrenadores);

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
