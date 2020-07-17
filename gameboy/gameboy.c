#include "gameboy.h"

void inicializar(){
	config = get_config();
	logger = crear_log();
}

int main(int argc, char** argv){

	if(argc < 4){
		printf("Parametros faltantes\n");
		exit(EXIT_FAILURE);
	}

	inicializar();

	char* destinatario = argv[1];
	queue_name cola = string_to_enum(argv[2]);
	if(cola == -1) cortar_ejecucion("Cola incorrecta");
	char** argumentos = argv + 3;
	char* argumentos_string = unir_args(argumentos, argc - 3);

	if(string_equals_ignore_case(destinatario, "TEAM")){

		if(cola != APPEARED_POKEMON){
			cortar_ejecucion("Cola inválida");
		}

		char* nombre_pokemon = string_new();
		uint32_t x, y;

		int parse_result = sscanf(argumentos_string, "%s %d %d", nombre_pokemon, &x, &y);

		if(parse_result < 3) cortar_ejecucion("Parametros incorrectos");

		uint32_t id = send_team(nombre_pokemon, x, y);
		if(id == -1) cortar_ejecucion("Error de conexion con el proceso TEAM");

	} else if(string_equals_ignore_case(destinatario, "BROKER")) {

		char nombre_pokemon[20];
		uint32_t x, y, cantidad, id, id_correlativo;
		int scan_result;

		switch(cola){
			case NEW_POKEMON:
				scan_result = sscanf(argumentos_string, "%s %d %d %d", nombre_pokemon, &x, &y, &cantidad);
				if(scan_result != 4) cortar_ejecucion("Parametros incorrectos");

				new_pokemon_msg* new_pok = new_msg(nombre_pokemon, x, y, cantidad);
				id = send_broker(cola, new_pok);
				break;

			case APPEARED_POKEMON:
				scan_result = sscanf(argumentos_string, "%s %d %d", nombre_pokemon, &x, &y);
				if(scan_result != 4) cortar_ejecucion("Parametros incorrectos");

				appeared_pokemon_msg* appeared_pok = appeared_msg(0, nombre_pokemon, x, y);
				id = send_broker(cola, appeared_pok);
				break;

			case CATCH_POKEMON:
				scan_result = sscanf(argumentos_string, "%s %d %d", nombre_pokemon, &x, &y);
				if(scan_result != 3) cortar_ejecucion("Parametros incorrectos");

				catch_pokemon_msg* catch_pok = catch_msg(nombre_pokemon, x, y);
				id = send_broker(cola, catch_pok);
				break;

			case CAUGHT_POKEMON:;
				uint32_t caught_status;
				scan_result = sscanf(argumentos_string, "%d %d", &id_correlativo, &caught_status);
				if (scan_result != 2) cortar_ejecucion("Parametros incorrectos");
				if(caught_status != 0 && caught_status != 1) cortar_ejecucion("Status de CAUGHT incorrecto");

				caught_pokemon_msg* caught_pok = caught_msg(id_correlativo, caught_status);
				id = send_broker(cola, caught_pok);
				break;

			case GET_POKEMON:
				scan_result = sscanf(argumentos_string, "%s", nombre_pokemon);
				if (scan_result != 1) cortar_ejecucion("Parametros incorrectos");

				get_pokemon_msg* get_pok = get_msg(nombre_pokemon);
				id = send_broker(cola, get_pok);
				break;

			default:
				cortar_ejecucion("Cola incorrecta");
		}

		if(id != -1){
			printf("ID asignado: %d\n", id);
		} else {
			cortar_ejecucion("Error de comunicación con el Broker");
		}

	} else if(string_equals_ignore_case(destinatario, "GAMECARD")){

		char nombre_pokemon[20];
		uint32_t x, y, cantidad, id_mensaje;
		int scan_result;

		switch(cola){
		case NEW_POKEMON:
			scan_result = sscanf(argumentos_string, "%s %d %d %d %d", nombre_pokemon, &x, &y, &cantidad, &id_mensaje);
			if(scan_result != 5) cortar_ejecucion("Parametros incorrectos");

			new_pokemon_msg* new_pok = new_msg(nombre_pokemon, x, y, cantidad);
			send_gamecard(cola, new_pok, id_mensaje);
			break;

		case CATCH_POKEMON:
			scan_result = sscanf(argumentos_string, "%s %d %d %d", nombre_pokemon, &x, &y, &id_mensaje);
			if(scan_result != 4) cortar_ejecucion("Parametros incorrectos");

			catch_pokemon_msg* catch_pok = catch_msg(nombre_pokemon, x, y);
			send_gamecard(cola, catch_pok, id_mensaje);
			break;

		case GET_POKEMON:
			scan_result = sscanf(argumentos_string, "%s %d", nombre_pokemon, &id_mensaje);
			if (scan_result != 2) cortar_ejecucion("Parametros incorrectos");

			get_pokemon_msg* get_pok = get_msg(nombre_pokemon);
			send_gamecard(cola, get_pok, id_mensaje);
			break;

		default:
			cortar_ejecucion("Cola incorrecta");
	}

	} else if(string_equals_ignore_case(destinatario, "SUSCRIPTOR")){

		int duracion_suscripcion;
		int scan_result = sscanf(argumentos_string, "%d", &duracion_suscripcion);
		if(scan_result != 1) cortar_ejecucion("Parametros incorrectos");

		char* ip_broker = config_get_string_value(config, "IP_BROKER");
		char* puerto_broker = config_get_string_value(config, "PUERTO_BROKER");
		int socket_broker = suscribirse_a_cola(cola, ip_broker, puerto_broker);
		free(ip_broker);
		free(puerto_broker);

		pthread_t listener_thread;
		pthread_create(&listener_thread, NULL, (void*) recibir_mensajes, (void*) &socket_broker);

		sleep(duracion_suscripcion);

		pthread_cancel(listener_thread);

	} else {
		cortar_ejecucion("Proceso inválido");
	}

	return EXIT_SUCCESS;
}
