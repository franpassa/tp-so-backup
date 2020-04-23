#ifndef ENTRENADOR_H_
#define ENTRENADOR_H_

#include "team.h"

uint32_t posicionXEntrenador(int nroEntrenador, char** posicionesEntrenadores);
uint32_t posicionYEntrenador(int nroEntrenador, char** posicionesEntrenadores);
void liberarArray(char** posicionesEntrenadores);
uint32_t cantidadTotalEntrenadores (char** posicionesEntrenadores);
t_list* insertarPokesEntrenador(uint32_t nroEntrenador, t_list* pokemons, char** pokesEntrenadores);

#endif /* ENTRENADOR_H_ */
