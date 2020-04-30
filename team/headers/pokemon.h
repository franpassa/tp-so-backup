
#ifndef HEADERS_POKEMON_H_
#define HEADERS_POKEMON_H_

#include "team.h"

uint32_t cantidadDePokemons(char* especie, t_list* lista);
t_list* crearListaObjetivoGlobal(t_list* pokesObjetivoGlobal);
void liberarPokemon(void* pokemon);


#endif /* HEADERS_POKEMON_H_ */
