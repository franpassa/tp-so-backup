#include "game_boy.h"

t_config* get_config(){
	t_config* mi_config = config_create(PATH_CONFIG);
	if(mi_config == NULL){
		printf("Error abriendo el archivo de configuración\n");
		exit(-1);
	}
	return mi_config;
}

t_log* crear_log(){
	t_log* mi_log = log_create(PATH_LOG, "Game Card", 0, LOG_LEVEL_INFO);
	if(mi_log == NULL){
		printf("Error creando el log\n");
		exit(-1);
	}
		return mi_log;
}

bool es_numerico(char* string){
	int largo_string = string_length(string);
	for(int i = 0; i < largo_string; i++){
		if(!isdigit(string[i])){
			return false;
		}
	}
	return true;
}

void cortar_ejecucion(char* mensaje_error){
	printf("ERROR: %s\n", mensaje_error);
	abort();
}

uint32_t send_team(char* nombre_pokemon, uint32_t X, uint32_t Y){

	char* ip_team = config_get_string_value(config, "IP_TEAM");
	char* puerto_team = config_get_string_value(config, "PUERTO_TEAM");

	appeared_pokemon_msg* msg = appeared_msg(nombre_pokemon, X, Y);
	uint32_t id = enviar_mensaje( ip_team, puerto_team, APPEARED_POKEMON, (void*) msg, false);

	return id;
}

uint32_t send_broker(queue_name cola, void* mensaje){

	char* ip_broker = config_get_string_value(config, "IP_BROKER");
	char* puerto_broker = config_get_string_value(config, "PUERTO_BROKER");

	uint32_t id = enviar_mensaje(ip_broker, puerto_broker, cola, mensaje, true);

	return id;
}

char* unir_args(char** args, int cant){
	char* args_string = string_new();

	for(int i = 0; i < cant; i++){
		string_append_with_format(&args_string, "%s ", args[i]);
	}

	string_trim(&args_string);

	return args_string;
}
