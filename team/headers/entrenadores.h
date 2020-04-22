#ifndef ENTRENADOR_H_
#define ENTRENADOR_H_

#include "team.h"

uint32_t posicionXEntrenador(int nroEntrenador, char** posicionesEntrenadores);
uint32_t posicionYEntrenador(int nroEntrenador, char** posicionesEntrenadores);
void liberarPosicionesEntrenadores(char** posicionesEntrenadores);
uint32_t cantidadTotalEntrenadores (char** posicionesEntrenadores);

#endif /* ENTRENADOR_H_ */