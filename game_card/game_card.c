#include "utils/game_card.h"

void inicializar(){
	config = get_config(CONFIG_PATH);
	logger = crear_log(LOG_PATH);
}

void terminar(){
	log_destroy(logger);
	config_destroy(config);
}

int main(){

	//inicializar();

	suscribirse_a_queue(CAUGHT_POKEMON,"127.0.0.1","6009");

	while(1);
/*
	char* punto_montaje = config_get_string_value(config, "PUNTO_MONTAJE_TALLGRASS");
	t_bitarray* bitarray = inicializar_filesystem(punto_montaje);
	print_bitarray(bitarray);
*/
	//terminar();

	return EXIT_SUCCESS;
}
