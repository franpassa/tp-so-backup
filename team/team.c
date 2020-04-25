#include "headers/team.h"

int main(){

	inicializarPrograma(); //Inicializo logger y config

	char** posicionesEntrenadores = config_get_array_value(config,"POSICIONES_ENTRENADORES");
	char** pokesEntrenadores = config_get_array_value(config, "POKEMON_ENTRENADORES");
	char** pokesObjetivos = config_get_array_value(config, "OBJETIVOS_ENTRENADORES");

	//t_list* entrenadores = list_create();

	//for(uint32_t i=0;i<cantidadTotalEntrenadores(posicionesEntrenadores);i++){
		t_entrenador* entrenador = malloc(sizeof(t_entrenador));
		entrenador->pokesAtrapados = list_create();
		entrenador->pokesObjetivos = list_create();

		entrenador->posicionX = posicionXEntrenador(0,posicionesEntrenadores);
		entrenador->posicionY = posicionYEntrenador(0,posicionesEntrenadores);
		entrenador->pokesAtrapados = insertarPokesEntrenador(0,entrenador->pokesAtrapados,pokesEntrenadores);
		entrenador->pokesObjetivos = insertarPokesEntrenador(0,entrenador->pokesObjetivos,pokesObjetivos);
		entrenador->idEntrenador = 0;

	//	list_add(entrenadores,entrenador);
	//}

	list_iterate(((t_entrenador*)entrenador)->pokesAtrapados,mostrarString);

	liberarEntrenador(entrenador);


	liberarArray(posicionesEntrenadores);
	liberarArray(pokesEntrenadores);
	liberarArray(pokesObjetivos);
	//list_destroy_and_destroy_elements(entrenadores,free);

	terminar_programa(); //Finalizo el programa

	return 0;
}

t_config* leer_config() {
	return config_create(PATH_CONFIG);
}

void terminar_programa(){
	log_destroy(logger);
	config_destroy(config);
}

void mostrarString(void *elemento){
  printf("%s\n", (char *)elemento);
}

void inicializarPrograma(){
	//Leo el archivo de configuracion
	config = config_create(PATH_CONFIG);
	printf("Archivo de configuracion leido.\n");

	//Inicializo el log
	logger = log_create(config_get_string_value(config,"LOG_FILE"), PROGRAM_NAME, 0, LOG_LEVEL_INFO);
	log_info(logger, "Log del proceso 'team' creado.");
	printf("Log del proceso 'team' creado.\n\n");
}
