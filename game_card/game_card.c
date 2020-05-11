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

	//inicializar();

	t_bitarray* bitarray = read_bitmap(fspaths->bitmap_file);
	print_bitarray(bitarray);
	set_bit(fspaths->bitmap_file, 2, true);
	t_bitarray* bitarray2 = read_bitmap(fspaths->bitmap_file);
	print_bitarray(bitarray2);

	terminar();

	return EXIT_SUCCESS;
}
