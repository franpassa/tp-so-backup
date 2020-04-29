#ifndef ENTRENADOR_H_
#define ENTRENADOR_H_

#include "team.h"

typedef struct Entrenador
{
	uint32_t posicionX;
	uint32_t posicionY;
	t_list* pokesAtrapados;
	t_list* pokesObjetivos;
	uint32_t idEntrenador;
}t_entrenador;

uint32_t posicionXEntrenador(int nroEntrenador, char** posicionesEntrenadores);
uint32_t posicionYEntrenador(int nroEntrenador, char** posicionesEntrenadores);
void liberarArray(char** posicionesEntrenadores);
uint32_t cantidadTotalEntrenadores (char** posicionesEntrenadores);
t_list* insertarPokesEntrenador(uint32_t nroEntrenador, t_list* pokemons, char** pokesEntrenadores);
void liberarEntrenador(void* entrenador);
void crearListaDeEntrenadores(t_list* entrenadores, char** posicionesEntrenadores, char** pokesEntrenadores, char** pokesObjetivos);
void crearListaPokesObjetivos(t_list* pokesObjetivos, t_list* entrenadores);

#endif /* ENTRENADOR_H_ */
