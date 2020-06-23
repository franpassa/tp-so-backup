#ifndef HEADERS_POKEMON_H_
#define HEADERS_POKEMON_H_

#include <conexiones.h>

#include <stdint.h>
#include <commons/collections/list.h>
#include <commons/string.h>
#include <pthread.h>


typedef struct Especie{
	char* especie;
	uint32_t cantidad;
}t_especie;

typedef struct Pokemon{
	char* nombre;
	uint32_t posicionX;
	uint32_t posicionY;
}t_pokemon;

t_list* pokemons_recibidos_historicos;
t_list* objetivos_posta; /*es una lista de especies*/
<<<<<<< HEAD
=======

pthread_mutex_t mutexPokemonsRecibidosHistoricos;
>>>>>>> 7a3b519ce534f7b5e92b4860df42e87b899b2ff0

// cantidades
uint32_t cantidadDePokemonsPorEspecie(char* especie, t_list* lista);

// listas nuevas / modificar listas
t_list* crearListaObjetivoGlobal(t_list* pokesObjetivoGlobal);
void agregarLocalizedRecibidoALista(t_list* pokemonsRecibidos, localized_pokemon_msg* pokemons);
void agregarAppearedRecibidoALista(t_list* pokemonsRecibidos, appeared_pokemon_msg* pokemon);

// liberar y mostrar
void liberarEspecie(void* especie);
void liberarPokemon(void* pokemon);
void mostrarPokemon(void* pokemon);
void mostrarEspecie(t_especie* especie);
void mostrarString(void*);


void igualarPokemons(t_pokemon* unPokemon, t_pokemon* otroPokemon);
void setearEnCeroPokemon(t_pokemon* unPokemon);
uint32_t distanciaEntrenadorPokemon(uint32_t posXEntrenador , uint32_t posYEntrenador, uint32_t posXPokemon, uint32_t posYPokemon);

bool sonIguales(t_list* pokemonsAtrapados,t_list* pokemonsObjetivos);
bool todosLosElementosDeLaPrimerListaEstanEnLaSegunda(t_list* listaA,t_list* listaB);
bool perteneceALaSegundaLista(char* unPokemon,t_list* listaDePokemons);
bool estaEnLaLista(char* unNombre, t_list* listadoDePokemons);
bool estaEnListaEspecie(char* pokemon, t_list* especies);
bool noSuperaElMaximoQuePuedoRecibir(char* pokemon);

#endif /* HEADERS_POKEMON_H_ */
