#include "headers/team.h"

int main()
{
	inicializarPrograma(); //Inicializo logger y config

	char** posicionesEntrenadores = config_get_array_value(config,"POSICIONES_ENTRENADORES");
	char** pokesEntrenadores = config_get_array_value(config, "POKEMON_ENTRENADORES");
	char** pokesObjetivos = config_get_array_value(config, "OBJETIVOS_ENTRENADORES");

	t_list* entrenadores = crearListaDeEntrenadores(posicionesEntrenadores,pokesEntrenadores,pokesObjetivos);

	t_list* pokesObjetivoGlobal = crearListaPokesObjetivos(entrenadores);

	t_list* listaObjetivos = crearListaObjetivoGlobal(pokesObjetivoGlobal);


	/* LIBERO ELEMENTOS */
	liberarArray(posicionesEntrenadores);
	liberarArray(pokesEntrenadores);
	liberarArray(pokesObjetivos);
	list_destroy_and_destroy_elements(pokesObjetivoGlobal, free);
	list_destroy_and_destroy_elements(entrenadores,liberarEntrenador);
	list_destroy_and_destroy_elements(listaObjetivos,liberarEspecie);


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



void inicializarPrograma(){
	//Leo el archivo de configuracion
	config = config_create(PATH_CONFIG);
	printf("Archivo de configuracion leido.\n");

	//Inicializo el log
	logger = log_create(config_get_string_value(config,"LOG_FILE"), PROGRAM_NAME, 0, LOG_LEVEL_INFO);
	log_info(logger, "Log del proceso 'team' creado.");
	printf("Log del proceso 'team' creado.\n\n");
}
