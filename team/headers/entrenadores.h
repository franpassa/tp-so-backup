#ifndef ENTRENADOR_H_
#define ENTRENADOR_H_

#include "pokemon.h"


typedef enum{
	MOTIVO_CATCH,
	MOTIVO_NADA,
	NADA
}motivo;

typedef struct Entrenador
{
	uint32_t posicionX;
	uint32_t posicionY;
	t_list* pokesAtrapados;
	t_list* pokesObjetivos;
	uint32_t idEntrenador;
	uint32_t idRecibido;
	t_pokemon* pokemonAMoverse;
	motivo motivoBloqueo;
}t_entrenador;



// posiciones
uint32_t posicionXEntrenador(int nroEntrenador, char** posicionesEntrenadores);
uint32_t posicionYEntrenador(int nroEntrenador, char** posicionesEntrenadores);

//cantidades
uint32_t cantidadTotalEntrenadores (char** posicionesEntrenadores);
uint32_t distanciaEntrenadorPokemon(uint32_t posXEntrenador , uint32_t posYEntrenador, uint32_t posXPokemon, uint32_t PosYPokemon);
/*
 * las funciones  el entrenadorMasCercanoAUnPOkemonDeLaLista y
 * elQueEstaMasCerca estan definidas en Team.h porque aca no reconocen el tipo
 * t_pokemon.
 * */

//liberaciones + muestra
void liberarArray(char** posicionesEntrenadores);
void liberarEntrenador(void* entrenador);
void mostrarEntrenador(void* entrenador);


// listas nuevas / modificar listas
t_list* insertarPokesEntrenador(uint32_t nroEntrenador, t_list* pokemons, char** pokesEntrenadores);
t_list* crearListaDeEntrenadores(char** posicionesEntrenadores, char** pokesEntrenadores, char** pokesObjetivos);
t_list* crearListaPokesObjetivos(t_list* entrenadores);
t_pokemon*  pokemonMasCercano (t_entrenador* unEntrenador, t_list* pokemons);
t_entrenador* entrenadorAPlanificar(t_list* listaEntrenadores, t_list* listaPokemons);

#endif /* ENTRENADOR_H_ */
