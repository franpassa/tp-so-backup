#include "headers/game_card.h"

void inicializar(){
	config = config_create(CONFIG_PATH);
	if(config == NULL){
		printf("Error abriendo el archivo de configuración\n");
		exit(-1);
	}

	logger = log_create(LOG_PATH, "Game Card", false, LOG_LEVEL_INFO);


}

void terminar(){
	log_destroy(logger);
	config_destroy(config);
}

int main(){

	inicializar();

	inicializar_bloques(config_get_string_value(config, "PUNTO_MONTAJE_TALLGRASS"), 10);

	terminar();

	return EXIT_SUCCESS;
}
