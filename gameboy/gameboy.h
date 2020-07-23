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
#include <commons/collections/list.h>
#include <conexiones.h>
#include <ctype.h>
#include <time.h>
#include <pthread.h>
#include <inttypes.h>

#define PROGRAM_NAME "Game Boy"
#define PATH_CONFIG "gameboy.config"
#define PATH_LOG "gameboy.log"

t_log* logger;
t_config* config;

typedef struct {
	int socket;
	queue_name cola;
} suscripcion_t;

suscripcion_t init_suscripcion(int socket, queue_name cola);

t_config* get_config();
t_log* crear_log();
void terminar_programa(int, t_log*, t_config*);
void cortar_ejecucion(char* mensaje_error);

// ------- Genericas -------

uint32_t send_team(char* nombre_pokemon, uint32_t X, uint32_t Y);
uint32_t send_gamecard(queue_name cola, void* mensaje, uint32_t id_mensaje);
char* unir_args(char** args, int cant);
bool es_numerico(char* string);
uint32_t send_broker(queue_name cola, void* mensaje);
void recibir_mensajes(queue_name* cola);
bool hay_que_confirmar();

#endif /* GAME_BOY_H_ */
