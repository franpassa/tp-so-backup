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

	t_pokemon* pikachu4 = malloc(sizeof(t_pokemon));
	pikachu4->nombre = string_new();	 //
	string_append(&pikachu4->nombre,"pikachu4");
	pikachu4->posicionX = 12;
	pikachu4->posicionY = 7;

	t_pokemon* pikachu2 = malloc(sizeof(t_pokemon));
	pikachu2->nombre = string_new();	 //
	string_append(&pikachu2->nombre,"pikachu2");
	pikachu2->posicionX = 2;
	pikachu2->posicionY = 5;

	t_list* pokemons = list_create();
	list_add(pokemons,pikachu4);
	//list_add(pokemons,pikachu2);

	t_entrenador* entrenadorBase = ((list_get(entrenadores,0)));

	printf("a = %d",(distanciaEntrenadorPokemon(entrenadorBase->posicionX,entrenadorBase->posicionY,pikachu2->posicionX,pikachu2->posicionY)));
	printf("a = %d",(distanciaEntrenadorPokemon(entrenadorBase->posicionX,entrenadorBase->posicionY,pikachu4->posicionX,pikachu4->posicionY)));

	mostrarPokemon(pokemonMasCercano((list_get(entrenadores,0)),pokemons));
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
