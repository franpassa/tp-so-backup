/*
 * Cliente.h
 *
 *  Created on: 28 feb. 2019
 *      Author: utnso
 */

#ifndef GAME_BOY_H_
#define GAME_BOY_H_

#include <stdio.h>
#include <stdlib.h>
#include <commons/log.h>
#include <commons/string.h>
#include <commons/config.h>
#include <conexiones.h>
#include <ctype.h>
#include <time.h>
#include <pthread.h>

#define PROGRAM_NAME "Game Boy"
#define PATH_CONFIG "game_boy.config"
#define PATH_LOG "game_boy.log"

t_log* logger;
t_config* config;

t_config* get_config();
t_log* crear_log();
void terminar_programa(int, t_log*, t_config*);
void cortar_ejecucion(char* mensaje_error);

// ------- Genericas -------

uint32_t send_team(char* nombre_pokemon, uint32_t X, uint32_t Y);
char* unir_args(char** args, int cant);
bool es_numerico(char* string);
uint32_t send_broker(queue_name cola, void* mensaje);
void recibir_mensajes(int* socket);

#endif /* GAME_BOY_H_ */
