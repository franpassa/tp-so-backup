#include "gameboy.h"

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

	appeared_pokemon_msg* msg = appeared_msg(0, nombre_pokemon, X, Y); // Hardcodeado el id_correlativo porque el team no lo necesita.

	return enviar_mensaje(ip_team, puerto_team, APPEARED_POKEMON, (void*) msg, 0, false);
}

uint32_t send_broker(queue_name cola, void* mensaje){

	char* ip_broker = config_get_string_value(config, "IP_BROKER");
	char* puerto_broker = config_get_string_value(config, "PUERTO_BROKER");

	return enviar_mensaje(ip_broker, puerto_broker, cola, mensaje, 0, true);
}

uint32_t send_gamecard(queue_name cola, void* mensaje, uint32_t id){

	char* ip_gamecard = config_get_string_value(config, "IP_GAMECARD");
	char* puerto_gamecard = config_get_string_value(config, "PUERTO_GAMECARD");

	return enviar_mensaje(ip_gamecard, puerto_gamecard, cola, mensaje, id, false);
}

char* unir_args(char** args, int cant){
	char* args_string = string_new();

	for(int i = 0; i < cant; i++){
		string_append_with_format(&args_string, "%s ", args[i]);
	}

	string_trim(&args_string);

	return args_string;
}

void recibir_mensajes(suscripcion_t* info_suscripcion){

	while(1){
		uint32_t id;
		queue_name tipo_msg;
		void* msg = recibir_mensaje(info_suscripcion->socket, &id, &tipo_msg);
		printf("id: %d ->", id);
		print_msg(info_suscripcion->cola, msg);
	}

}

suscripcion_t init_suscripcion(int socket, queue_name cola){
	suscripcion_t suscripcion;
	suscripcion.socket = socket;
	suscripcion.cola = cola;

	return suscripcion;
}
