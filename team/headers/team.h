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

void terminar_programa(int conexion, t_log* logger, t_config* config);
t_config* leer_config(void);
t_log* iniciar_logger(void);


