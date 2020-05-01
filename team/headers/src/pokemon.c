#include "../pokemon.h"

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
