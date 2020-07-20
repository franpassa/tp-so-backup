//#ifndef TEAM_H_
//#define TEAM_H_

#define PROGRAM_NAME "team"
#define PATH_CONFIG "team.config"
#define PATH_LOG "team.log"


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
#include <semaphore.h>

// conexiones
#include <conexiones.h>
#include "entrenadores.h"
#include "conexiones_team.h"

//VARIABLES GLOBALES



// variables globales sacadas del config
t_config* config;
char* ip_broker;
char* puerto_broker;
char* IP;
char* PUERTO;
char** posicionesEntrenadores;
char** pokesEntrenadores;
char** pokesObjetivos;
uint32_t tiempo_reconexion;
uint32_t socket_caught;
uint32_t socket_localized;
uint32_t socket_appeared;


t_list* estado_exit; /*lista de entrenadores*/
t_list* objetivos_globales;/*es una lista de especies*/
t_list* ids_enviados;
t_list* pokemons_objetivos;

pthread_mutex_t mutexEstadoExit;
pthread_mutex_t mutexIdsEnviados;
pthread_mutex_t mutexReconexion;
pthread_mutex_t mutexLog;
pthread_mutex_t mutexHayEntrenadorProcesando;
pthread_mutex_t mutexCantidadDeadlocks;
pthread_mutex_t mutexEspera;
pthread_cond_t cond_reconectado;

pthread_t hilo_escucha;
pthread_t hilo_recibir_localized;
pthread_t hilo_recibir_caught;
pthread_t hilo_recibir_appeared;
pthread_t hilo_estado_exec;
pthread_t hilo_pasar_a_ready;
pthread_t hilo_deadlock;
pthread_t hilo_invocador_escucha;

bool hayEntrenadorProcesando; 	// funcion a futuro, chequear en otro proceso si hay un entrenador
bool envioGets;								//corriendo porque cuando sale del bloqueo porque ya le							// mandaron el caught

//FUNCIONES

t_log* crear_log();
t_config* leer_config();
void inicializarPrograma();
void liberar_recursos();
void estado_exec();
void inicializarVariables();
void liberarVariables();


//#endif /* TEAM_H_ */
