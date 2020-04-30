#include "game_card.h"

t_bitarray* inicializar_filesystem(char* punto_montaje){

}

void inicializar_bloques(char* punto_montaje, int cantidad){

	char* blocks_path = string_new();
	string_append_with_format(&blocks_path, "%s/Blocks", punto_montaje);

	FILE* block_file;

	for(int i = 0; i < cantidad; i++){
		char* block_path = string_new();
		string_append_with_format(&block_path, "%s/%d.bin", blocks_path, i);

		block_file = fopen(block_path, "wb");

		if(block_file == NULL){
			log_error(logger, "Error creando el bloque '%s'", block_path);
			exit(1);
		}

		fclose(block_file);

		free(block_path);
	}

	free(blocks_path);
}
