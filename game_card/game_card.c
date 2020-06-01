#include "utils/game_card.h"

void inicializar(){
	config = get_config(CONFIG_PATH);
	logger = crear_log(LOG_PATH);

	char* punto_montaje = config_get_string_value(config, "PUNTO_MONTAJE_TALLGRASS");
	fspaths = init_fspaths(punto_montaje);
	init_fs();
}

void terminar(){
	log_destroy(logger);
	config_destroy(config);
	free_fspaths(fspaths);
}

int main(){

	inicializar();
	t_pokemon pikachu = init_pokemon("eee", 3, 2, 1);
	crear_pokemon(pikachu);
	terminar();

	return EXIT_SUCCESS;
}
