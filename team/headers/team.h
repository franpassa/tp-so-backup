//#ifndef TEAM_H_
//#define TEAM_H_

#define PROGRAM_NAME "team"
#define PATH_CONFIG "/home/utnso/workspace/tp-2020-1c-Cuarenteam/team/team.config"
#define PATH_LOG "/home/utnso/workspace/tp-2020-1c-Cuarenteam/team/team.log"



// general
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
#include <time.h>
#include <pthread.h>
#include <math.h>

// conexiones
#include <conexiones.h>
#include "conexiones_team.h"

// interno

#include "entrenadores.h"
#include "pokemon.h"

t_log* logger;
t_config* config;

void inicializarPrograma();
void terminar_programa();
t_config* leer_config();
void mostrarString(void*);



//#endif /* TEAM_H_ */
