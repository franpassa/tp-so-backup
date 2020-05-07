#ifndef TEAM_H_
#define TEAM_H_

#define PROGRAM_NAME "team"
#define PATH_CONFIG "/home/utnso/workspace/tp-2020-1c-Cuarenteam/team/team.config"
#define PATH_LOG "/home/utnso/workspace/tp-2020-1c-Cuarenteam/team/team.log"


#include <stdio.h>
#include <stdlib.h>
#include <commons/string.h>
#include <commons/config.h>
#include <commons/collections/list.h>
#include <commons/log.h>
#include <readline/readline.h>
#include <signal.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <stdint.h>
#include <conexiones.h>
#include <time.h>
#include <pthread.h>
#include "math.h"
#include "entrenadores.h"
#include "pokemon.h"
#include "conexiones_team.h"


typedef enum{
	MOTIVO_CATCH,
	MOTIVO_NADA
}motivo;

typedef struct Especie{
	char* especie;
	uint32_t cantidad;
}t_especie;

typedef struct Pokemon{
	char* nombre;
	uint32_t posicionX;
	uint32_t posicionY;
}t_pokemon;


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


t_log* logger;
t_config* config;

void inicializarPrograma();
void terminar_programa();
t_config* leer_config();
void mostrarString(void*);
uint32_t cantidadDePokemons(char* especie, t_list* lista);
t_list* crearListaObjetivoGlobal(t_list* pokesObjetivoGlobal);
void agregarPokemonsRecibidosALista(t_list* pokemonsRecibidos, localized_pokemon_msg* pokemons);

t_entrenador* elEntrenadorMasCercanoAUnPokemonDeLaLista (t_list* listadoDeEntrenadores,t_list* listadoDePokemones);
t_entrenador* elQueEstaMasCerca(void* entrenador1, void* entrenador2,void* pokemon);






#endif /* TEAM_H_ */
