#ifndef HEADERS_POKEMON_H_
#define HEADERS_POKEMON_H_

#include <conexiones.h>

#include <stdint.h>
#include <commons/collections/list.h>


typedef struct Especie{
	char* especie;
	uint32_t cantidad;
}t_especie;

typedef struct Pokemon{
	char* nombre;
	uint32_t posicionX;
	uint32_t posicionY;
}t_pokemon;


// cantidades
uint32_t cantidadDePokemonsPorEspecie(char* especie, t_list* lista);

// listas nuevas / modificar listas
t_list* crearListaObjetivoGlobal(t_list* pokesObjetivoGlobal);
void agregarPokemonsRecibidosALista(t_list* pokemonsRecibidos, localized_pokemon_msg* pokemons);

// liberar y mostrar
void liberarEspecie(void* especie);
void liberarPokemon(void* pokemon);
void mostrarPokemon(void* pokemon);

void igualarPokemons(t_pokemon* unPokemon, t_pokemon* otroPokemon);
void setearEnCeroPokemon(t_pokemon* unPokemon);
uint32_t distanciaEntrenadorPokemon(uint32_t posXEntrenador , uint32_t posYEntrenador, uint32_t posXPokemon, uint32_t posYPokemon);



#endif /* HEADERS_POKEMON_H_ */
