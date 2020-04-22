#ifndef TEAM_H_
#define TEAM_H_

#define PROGRAM_NAME "team"
#define PATH_CONFIG "team.config"
#define PATH_LOG "team.log"

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

typedef struct Entrenador{
	uint32_t posicion[2];
	t_list* pokesAtrapados;
	t_list* pokesObjetivos;
	uint32_t idEntrenador;
}t_entrenador;

t_log* logger;
t_config* config;

void inicializarPrograma(void);
void terminar_programa(void);
t_config* leer_config(void);
t_log* iniciar_logger(void);

#endif /* TEAM_H_ */
