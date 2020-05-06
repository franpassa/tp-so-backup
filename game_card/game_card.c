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

	inicializar();

	// inicializar_bloques(config_get_string_value(config, "PUNTO_MONTAJE_TALLGRASS"), 10);
	uint32_t pares_coordenadas[4] = {1, 3, 5, 6};
	localized_pokemon_msg* msg = localized_msg(21, "pikachu", 2, pares_coordenadas);

	terminar();

	return EXIT_SUCCESS;
}
