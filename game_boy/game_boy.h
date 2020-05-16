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

queue_name string_to_enum(char* string);
uint32_t send_team(char* nombre_pokemon, uint32_t X, uint32_t Y);
bool es_numerico(char* string);
int parse_team_args(char** arguments, char** nombre_pokemon, uint32_t* x, uint32_t* y);

#endif /* GAME_BOY_H_ */
