#include "utils/game_card.h"

int* manuela;

void inicializar(){
	config = get_config(CONFIG_PATH);
	logger = crear_log(LOG_PATH);
}

void terminar(){
	log_destroy(logger);
	config_destroy(config);
}

int main(){

	inicializar();

	inicializar_filesystem(config_get_string_value(config, "PUNTO_MONTAJE_TALLGRASS"));

	terminar();

	return EXIT_SUCCESS;
}
