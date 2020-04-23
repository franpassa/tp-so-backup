#include "headers/team.h"

int main(){

	inicializarPrograma(); //Inicializo logger y config

	/*PRUEBA POSICIONES ENTRENADORES*/
	char** posicionesEntrenadores = config_get_array_value(config,"POSICIONES_ENTRENADORES");

	uint32_t cant = cantidadTotalEntrenadores(posicionesEntrenadores);
	uint32_t posX = posicionXEntrenador(0,posicionesEntrenadores);
	uint32_t posY = posicionYEntrenador(0,posicionesEntrenadores);

	printf("La cantidad total de entrenadores es: %d\n\n", cant);
	printf("La posicion en X del entrenador 0 es: %d\n\n", posX);
	printf("La posicion en Y del entrenador 0 es: %d\n\n", posY);

	liberarArray(posicionesEntrenadores);

	/*PRUEBA POKEMONES ENTRENADORES*/

	char** pokesEntrenadores = config_get_array_value(config, "POKEMON_ENTRENADORES");
	t_list* pokemons = list_create();

	pokemons = insertarPokesEntrenador(0,pokemons,pokesEntrenadores);

	printf("Los pokemons del entrenador 0 son: \n");
	list_iterate(pokemons,mostrarString); // Itero la funcion "mostrarString" en la lista

	list_destroy_and_destroy_elements(pokemons,free); // Libero la lista de pokemons del entrenador

	liberarArray(pokesEntrenadores);

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
