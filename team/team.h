#define PROGRAM_NAME "team"
#define PATH_CONFIG "team.config"
#define PATH_LOG "team.log"

#include<stdio.h>
#include<stdlib.h>
#include<commons/string.h>
#include<commons/config.h>
#include<commons/log.h>
#include<readline/readline.h>
#include<signal.h>
#include<unistd.h>
#include<sys/socket.h>
#include<netdb.h>
#include<string.h>
#include "headers/team_log.h"
#include "headers/team_config.h"

void terminar_programa(int conexion, t_log* logger, t_config* config);

void terminar_programa(int conexion, t_log* logger, t_config* config){
	close(conexion);
	log_destroy(logger);
	config_destroy(config);

}
