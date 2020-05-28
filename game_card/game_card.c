#include "utils/game_card.h"

void inicializar(){
	config = get_config(CONFIG_PATH);
	logger = crear_log(LOG_PATH);

	char* punto_montaje = config_get_string_value(config, "PUNTO_MONTAJE_TALLGRASS");
	fspaths = init_fspaths(punto_montaje);
	init_fs(fspaths);
}

void terminar(){
	log_destroy(logger);
	config_destroy(config);
}


int main(){


	/*int socket_broker = suscribirse_a_cola(NEW_POKEMON, "127.0.0.1", "6009");
	int socket_broker2 = suscribirse_a_cola(NEW_POKEMON, "127.0.0.1", "6009");

	new_pokemon_msg* msg = new_msg("Hola soy Pikachu", 3, 4, 10);
	uint32_t id = enviar_mensaje("127.0.0.1","6009",NEW_POKEMON, (void*) msg,true);
	printf("id recibido: %d\n", id);

	uint32_t id_recibido;
	new_pokemon_msg* msg_recibido = (new_pokemon_msg*) recibir_mensaje(NEW_POKEMON, socket_broker, &id_recibido);

	new_pokemon_msg* msg2 = new_msg("Hola soy Squirtle", 3, 4, 10);
	uint32_t id2 = enviar_mensaje("127.0.0.1","6009",NEW_POKEMON, (void*) msg2,true);
	printf("id recibido: %d\n", id2);

	uint32_t id_recibido2;
	new_pokemon_msg* msg_recibido2 = (new_pokemon_msg*) recibir_mensaje(NEW_POKEMON, socket_broker2, &id_recibido2);

	print_msg(NEW_POKEMON, (void*) msg_recibido);
	print_msg(NEW_POKEMON, (void*) msg_recibido2);


	while(1);*/


	return EXIT_SUCCESS;
}
