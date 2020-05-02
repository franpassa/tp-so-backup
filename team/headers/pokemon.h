
#ifndef HEADERS_POKEMON_H_
#define HEADERS_POKEMON_H_

#include "team.h"
#include <conexiones.h>

typedef struct Especie{
	char* especie;
	uint32_t cantidad;
}t_especie;

typedef struct Pokemon{
	char* nombre;
	uint32_t posicionX;
	uint32_t posicionY;
}t_pokemon;

uint32_t cantidadDePokemonsPorEspecie(char* especie, t_list* lista);
t_list* crearListaObjetivoGlobal(t_list* pokesObjetivoGlobal);
void liberarEspecie(void* especie);
void agregarPokemonsRecibidosALista(t_list* pokemonsRecibidos, localized_pokemon_msg* pokemons);


#endif /* HEADERS_POKEMON_H_ */
