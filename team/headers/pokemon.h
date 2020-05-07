#ifndef HEADERS_POKEMON_H_
#define HEADERS_POKEMON_H_

#include "team.h"

// cantidades
uint32_t cantidadDePokemonsPorEspecie(char* especie, t_list* lista);

// listas nuevas / modificar listas
t_list* crearListaObjetivoGlobal(t_list* pokesObjetivoGlobal);
void agregarPokemonsRecibidosALista(t_list* pokemonsRecibidos, localized_pokemon_msg* pokemons);

// liberar y mostrar
void liberarEspecie(void* especie);
void liberarPokemon(void* pokemon);
void mostrarPokemon(void* pokemon);


#endif /* HEADERS_POKEMON_H_ */
