//#ifndef TEAM_H_
//#define TEAM_H_

#define PROGRAM_NAME "team"
#define PATH_CONFIG "/home/utnso/workspace/tp-2020-1c-Cuarenteam/team/team.config"
#define PATH_LOG "/home/utnso/workspace/tp-2020-1c-Cuarenteam/team/team.log"
#define IP "127.0.0.1"
#define PUERTO "1090"


// general
#include <stdio.h>
#include <stdlib.h>
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
#include <time.h>
#include <pthread.h>
#include <math.h>

// conexiones
#include <conexiones.h>
#include "conexiones_team.h"

// interno

#include "entrenadores.h"

//VARIABLES GLOBALES

t_log* logger;
t_config* config;

t_list* estado_new;
t_list* estado_bloqueado;
t_list* estado_ready;
t_list* estado_exit;

t_list* ids_get; //Lista de IDs de los mensajes GET_POKEMON
t_list* pokemons_recibidos;

localized_pokemon_msg* mensaje_recibido_localized;

pthread_t planificarEntrenador;
pthread_mutex_t mutexEstadoExec;
pthread_t recibir_localized;
pthread_t hilo_escucha;

int ciclosConsumidos;
int socket_localized;
int socket_caught;
int socket_appeared;

//FUNCIONES

t_log* crear_log();
t_config* leer_config();
void inicializarPrograma();
void terminar_programa();
void* estado_exec(void* unEntrenador);


//#endif /* TEAM_H_ */
