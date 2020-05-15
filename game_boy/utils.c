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

}

queue_name string_to_enum(char* string){

	if(string_equals_ignore_case(string, "NEW_POKEMON")){
		return NEW_POKEMON;
	} else if(string_equals_ignore_case(string, "APPEARED_POKEMON")) {
		return APPEARED_POKEMON;
	} else if(string_equals_ignore_case(string, "CATCH_POKEMON")){
		return CATCH_POKEMON;
	} else if(string_equals_ignore_case(string, "CAUGHT_POKEMON")){
		return CAUGHT_POKEMON;
	} else if(string_equals_ignore_case(string, "GET_POKEMON")){
		return GET_POKEMON;
	} else if(string_equals_ignore_case(string, "LOCALIZED_POKEMON")){
		return LOCALIZED_POKEMON;
	}
	return -1;
}

void cortar_ejecucion(char* mensaje_error){
	printf("ERROR: %s\n", mensaje_error);
	exit(EXIT_FAILURE);
}

void enviar_a_broker(){

}

void send_team(char* nombre_pokemon, uint32_t X, uint32_t Y){

	char* ip_team = config_get_string_value(config, "IP_TEAM");
	char* puerto_team = config_get_string_value(config, "PUERTO_TEAM");

	int socket_team = conectar_como_productor(ip_team, puerto_team);

	appeared_pokemon_msg* msg = appeared_msg(nombre_pokemon, X, Y);
	uint32_t id = enviar_mensaje(APPEARED_POKEMON, (void*) msg, socket_team);
	printf("APPEARED enviado - ID %d\n", id);

}
