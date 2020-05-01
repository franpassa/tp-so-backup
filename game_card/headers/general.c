#include "game_card.h"

t_config* get_config(char* config_path){
	t_config* mi_config = config_create(config_path);
	if(mi_config == NULL){
		printf("Error abriendo el archivo de configuración\n");
		exit(-1);
	}
	return mi_config;
}

t_log* crear_log(char* log_path){
	t_log* mi_log = log_create(log_path, "Game Card", 0, LOG_LEVEL_INFO);
	if(mi_log == NULL){
		printf("Error creando el log\n");
		exit(-1);
	}
		return mi_log;
}
