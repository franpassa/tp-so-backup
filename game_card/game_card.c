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

	return EXIT_SUCCESS;
}
