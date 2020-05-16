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

		char* nombre_pokemon = NULL;
		uint32_t x, y;

		int parse_result = parse_team_args(argumentos, &nombre_pokemon, &x, &y);

		if(parse_result == -1) cortar_ejecucion("Parametros incorrectos");

		uint32_t id = send_team(nombre_pokemon, x, y);
		if(id == -1){
			printf("Error de conexión con el team");
		} else {
			printf("APPEARED_POKEMON enviado - ID %d\n", id);
		}


	} else if(string_equals_ignore_case(destinatario, "BROKER")) {

		/*queue_name cola = string_to_enum(cola_string);

		switch(cola){
		case NEW_POKEMON:

		}

		enviar_a_broker(cola, )*/

	} else if(string_equals_ignore_case(destinatario, "GAMECARD")){

	} else if(string_equals_ignore_case(destinatario, "SUSCRIPTOR")){

	} else {
		cortar_ejecucion("Proceso inválido");
	}

	return EXIT_SUCCESS;
}
