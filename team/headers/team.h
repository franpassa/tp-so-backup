#ifndef TEAM_H_
#define TEAM_H_

#define PROGRAM_NAME "team"
#define PATH_CONFIG "/home/utnso/workspace/tp-2020-1c-Cuarenteam/team/team.config"
#define PATH_LOG "/home/utnso/workspace/tp-2020-1c-Cuarenteam/team/team.log"

#include<stdio.h>
#include<stdlib.h>
#include<commons/string.h>
#include<commons/config.h>
#include<commons/collections/list.h>
#include<commons/log.h>
#include<readline/readline.h>
#include<signal.h>
#include<unistd.h>
#include<sys/socket.h>
#include<netdb.h>
#include<string.h>
#include<stdint.h>
#include "entrenadores.h"
#include "pokemon.h"

t_log* logger;
t_config* config;

typedef struct Especie{
	char* especie;
	uint32_t cantidad;
}t_especie;

void inicializarPrograma();
void terminar_programa();
t_config* leer_config();
void mostrarString(void*);
uint32_t cantidadDePokemons(char* especie, t_list* lista);
t_list* crearListaObjetivoGlobal(t_list* pokesObjetivoGlobal);
void liberarPokemon(void* pokemon);

#endif /* TEAM_H_ */
