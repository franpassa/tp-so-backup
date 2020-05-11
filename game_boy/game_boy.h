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
#include <readline/readline.h>

#define PROGRAM_NAME "Game Boy"
#define PATH_CONFIG "game_boy.config"
#define PATH_LOG "game_boy.log"


t_log* iniciar_logger(void);
t_config* leer_config(void);
void terminar_programa(int, t_log*, t_config*);

#endif /* GAME_BOY_H_ */
