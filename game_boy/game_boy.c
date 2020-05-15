#include "game_boy.h"

void inicializar(){
	config = get_config();
	logger = crear_log();
}

int main(int argc, char** argv){

	if(argc < 4){
		printf("Parámetros faltantes\n");
		exit(EXIT_FAILURE);
	}

	inicializar();

	char* destinatario = argv[1];
	char* cola_string = argv[2];
	char** argumentos = argv + 3;

	if(string_equals_ignore_case(destinatario, "TEAM")){

		queue_name cola = string_to_enum(cola_string);

		if(cola != APPEARED_POKEMON){
			cortar_ejecucion("Cola inválida");
		}

		char* nombre_pokemon = argumentos[0];
		if(es_numerico(nombre_pokemon)) cortar_ejecucion("nombre de pokemon con numeros");

		char* endptr;
		uint32_t x = strtol(argumentos[1], &endptr, 10);
		uint32_t y = strtol(argumentos[2], &endptr, 10);

		if(x == 0 || y == 0) cortar_ejecucion("coordenadas no numericas");
		send_team(nombre_pokemon, x, y);

	} else if(string_equals_ignore_case(destinatario, "BROKER")) {

		queue_name cola = string_to_enum(cola_string);

		if(cola == -1){
			cortar_ejecucion("Cola inválida");
		}

		//enviar_a_broker(cola, )

	} else if(string_equals_ignore_case(destinatario, "GAMECARD")){

	} else if(string_equals_ignore_case(destinatario, "SUSCRIPTOR")){

	} else {
		cortar_ejecucion("Proceso inválido");
	}

	return EXIT_SUCCESS;
}
