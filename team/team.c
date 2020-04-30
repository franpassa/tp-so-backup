#include "headers/team.h"

int main(){

	inicializarPrograma(); //Inicializo logger y config

	char** posicionesEntrenadores = config_get_array_value(config,"POSICIONES_ENTRENADORES");
	char** pokesEntrenadores = config_get_array_value(config, "POKEMON_ENTRENADORES");
	char** pokesObjetivos = config_get_array_value(config, "OBJETIVOS_ENTRENADORES");

	t_list* entrenadores = list_create();

	crearListaDeEntrenadores(entrenadores,posicionesEntrenadores,pokesEntrenadores,pokesObjetivos);

	t_list* pokesObjetivoGlobal = crearListaPokesObjetivos(entrenadores);

	list_iterate(pokesObjetivoGlobal,mostrarString);

	uint32_t cantidad = cantidadDePokemons("Squirtle",pokesObjetivoGlobal);

	printf("\nLa cantidad son: %d\n\n",cantidad);

	t_list* listaObjetivos = crearListaObjetivoGlobal(pokesObjetivoGlobal);

	printf("La cantidad de pokemons sin repetir son: %d\n",list_size(listaObjetivos));
	printf("El pokemon de nombre %s",((t_especie*) list_get(listaObjetivos,1))->especie);
	printf(" aparece %d veces\n",((t_especie*)list_get(listaObjetivos,1))->cantidad);


	/* LIBERO ELEMENTOS */
	liberarArray(posicionesEntrenadores);
	liberarArray(pokesEntrenadores);
	liberarArray(pokesObjetivos);
	list_destroy_and_destroy_elements(pokesObjetivoGlobal, free);
	list_destroy_and_destroy_elements(entrenadores,liberarEntrenador);
	list_destroy_and_destroy_elements(listaObjetivos,liberarPokemon);

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

uint32_t cantidadDePokemons(char* especie, t_list* lista){
	uint32_t cantidad = 0;

	for(uint32_t i=0;i<list_size(lista);i++){
		if((strcmp(especie,(char*)list_get(lista,i)))==0){
			cantidad++;
		}
	}
	return cantidad;
}

t_list* crearListaObjetivoGlobal(t_list* pokesObjetivoGlobal){

	t_list* objetivoGlobal = list_create();

	for(int i=0; i < list_size(pokesObjetivoGlobal); i++){

		char* pokemon = (char*) list_get(pokesObjetivoGlobal, i);

		bool es_el_mismo(t_especie* especie){
			return string_equals_ignore_case(pokemon, especie->especie);
		}

		t_especie* pokemon_ya_en_lista = list_find(objetivoGlobal, (void*) es_el_mismo);

		if(pokemon_ya_en_lista == NULL){

			char* copia_pokemon = string_duplicate(pokemon);
			t_especie* nueva_especie = malloc(sizeof(t_especie));
			nueva_especie->especie = copia_pokemon;
			nueva_especie->cantidad = 1;
			list_add(objetivoGlobal,nueva_especie);
		} else {
			pokemon_ya_en_lista->cantidad++;
		}

	}

	return objetivoGlobal;
}


void liberarPokemon(void* pokemon){
	t_especie* liberar = (t_especie*)pokemon;
	free(liberar->especie);
	free(liberar);
}
