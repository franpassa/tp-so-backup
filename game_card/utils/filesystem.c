#include "../utils/game_card.h"

/*
 * Esta función chequea si existe el archivo Bitmap.bin,
 * en caso que no se asume que se debe crear la estructura
 * del filesystem desde cero y retorna el bitarray correspondiente.
*/
t_bitarray* inicializar_filesystem(char* punto_montaje){
	t_bitarray* bitarray = NULL;

	// TENGO QUE VERIFICAR QUE EXISTA LA CARPETA 'METADATA' O EL BITMAP.BIN?

	/*char* metadata_path = string_new();
	string_append_with_format(&metadata_path, "%s/Metadata", punto_montaje);

	DIR* metadata_dir = opendir(metadata_path);*/

	char* bitmap_path = string_new();
	string_append_with_format(&bitmap_path, "%s/Metadata/Bitmap.bin", punto_montaje);
	FILE* bitmap_file = fopen(bitmap_path, "r");

	if(bitmap_file){
		// Caso metadata existente
		fclose(bitmap_file);
		bitarray = leer_bitmap(bitmap_path);

	} else if (ENOENT == errno){
		// Caso metadata NO existente
		char* path_config = string_new();
		string_append_with_format(&path_config, "%s/Metadata/Metadata.bin", punto_montaje);

		t_config* config_metadata = config_create(path_config);
		int bloques = config_get_int_value(config_metadata, "BLOCKS");

		bitarray = crear_bitmap(bitmap_path, bloques);


	} else {
		// Otro error, finalizar proceso
		terminar_aplicacion("Error desconocido al leer directorio de metadata");
	}

	free(bitmap_path);
	return bitarray;
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

void print_bitarray(t_bitarray* bitarray){
	size_t bitarray_size = bitarray_get_max_bit(bitarray);
	char* bits_string = string_new();
	int j = 0;
	for(int i = 0; i < bitarray_size; i++){
		j++;
		bool bit = bitarray_test_bit(bitarray, i);
		if(bit){
			string_append(&bits_string, "1");
		} else {
			string_append(&bits_string, "0");
		}
	}

	printf("bitarray de largo %d: %s\n", j, bits_string);
	free(bits_string);
}

t_bitarray* leer_bitmap(char* bitmap_path){
	t_bitarray* bitarray = NULL;

	FILE* bitmap_file_ptr = fopen(bitmap_path, "r");

	if(bitmap_file_ptr == NULL){
		char* mensaje_error = string_new();
		string_append_with_format(&mensaje_error, "No existe el archivo %s", bitmap_path);
		terminar_aplicacion(mensaje_error);
	}

	// Obtengo tamaño de bitmap
	fseek(bitmap_file_ptr, 0, SEEK_END);
	long file_size = ftell(bitmap_file_ptr);
	fseek(bitmap_file_ptr, 0, SEEK_SET);

	char* buffer = calloc(file_size, sizeof(char));
	long bytes_read = fread(buffer, sizeof(char), file_size, bitmap_file_ptr);

	if(file_size == bytes_read){
		int size_in_bytes = file_size / 8;
		char* data = malloc(size_in_bytes);
		bitarray = bitarray_create_with_mode(data, size_in_bytes, LSB_FIRST);

		for(int i = 0; i < file_size; i++){
			char bit = buffer[i];

			if(bit == '1'){
				bitarray_set_bit(bitarray, i);
			} else if(bit == '0') {
				bitarray_clean_bit(bitarray, i);
			} else {
				terminar_aplicacion("caracter erroneo en bitmap");
			}
		}
		free(buffer);
		fclose(bitmap_file_ptr);

	} else {
		terminar_aplicacion("Error leyendo bitmap");
	}

	return bitarray;
}

t_bitarray* crear_bitmap(char* file_path, int cantidad_bloques){
	t_bitarray* bitarray = NULL;

	FILE* bitmap_file = fopen(file_path, "w");

	if(bitmap_file){
		char* string_bitarray = string_repeat('0', cantidad_bloques);

		fputs(string_bitarray, bitmap_file);
		free(string_bitarray);

		fclose(bitmap_file);

		int tam_bytes = cantidad_bloques/8;
		char* bits = calloc(tam_bytes, sizeof(char));
		bitarray = bitarray_create_with_mode(bits, tam_bytes, LSB_FIRST);
	} else {
		terminar_aplicacion("Error creando bitmap");
	}

	return bitarray;
}

void crear_bloques(char* path, int cantidad_bloques){

	FILE* bloque;

	for(int i = 0; i < cantidad_bloques; i++){
		char* nombre_bloque = string_new();
		string_append_with_format(&nombre_bloque, "%d.bin", i);
		bloque = fopen(nombre_bloque, "w");
		free(nombre_bloque);
		fclose(bloque);
	}
}
