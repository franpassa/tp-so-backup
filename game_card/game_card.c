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

	//inicializar_filesystem(config_get_string_value(config, "PUNTO_MONTAJE_TALLGRASS"));
	char* path = "/home/utnso/Escritorio/Bitmap.bin";
	t_bitarray* bitarray = crear_bitmap(path, 128 * 8);
	print_bitarray(bitarray);

	terminar();

	return EXIT_SUCCESS;
}
