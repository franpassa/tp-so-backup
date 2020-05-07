#include "../pokemon.h"

uint32_t cantidadDePokemonsPorEspecie(char* especie, t_list* lista){
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

void agregarPokemonsRecibidosALista(t_list* pokemonsRecibidos, localized_pokemon_msg* pokemons){
	int j = 1;
	for(int i=0; i<(pokemons->cantidad_posiciones)*2; i+=2){
			t_pokemon* pokemon = malloc(sizeof(t_pokemon));
			char* copia_nombre = string_duplicate(pokemons->nombre_pokemon);
			pokemon->nombre = copia_nombre;
			pokemon->posicionX = pokemons->pares_coordenadas[i];
			pokemon->posicionY = pokemons->pares_coordenadas[j];
			list_add(pokemonsRecibidos,pokemon);
			j+=2;
	}
}

void mostrarPokemon(void* pokemon)
{
    printf("El nombre del pokemon es: ");
    mostrarString((((t_pokemon*)pokemon)->nombre));
    printf("La pos en X es: %d\n",((t_pokemon*)pokemon)->posicionX);
    printf("La pos en y es: %d\n",((t_pokemon*)pokemon)->posicionY);
}

void liberarEspecie(void* especie){
	t_especie* liberar = (t_especie*)especie;
	free(liberar->especie);
	free(liberar);
}

void liberarPokemon(void* pokemon){
	t_pokemon* liberar = (t_pokemon*)pokemon;
	free(liberar->nombre);
	free(liberar);
}
