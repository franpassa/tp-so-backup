#include "../utils/game_card.h"

t_fspaths* init_fspaths(char* punto_montaje){
	char* bitmap_path = string_new();
	char* metadata_path = string_new();
	char* blocks_folder = string_new();
	char* files_folder = string_new();

	string_append_with_format(&bitmap_path, "%s/Metadata/Bitmap.bin", punto_montaje);
	string_append_with_format(&metadata_path, "%s/Metadata/Metadata.bin", punto_montaje);
	string_append_with_format(&blocks_folder, "%s/Blocks", punto_montaje);
	string_append_with_format(&files_folder, "%s/Files", punto_montaje);

	t_fspaths* paths = malloc(sizeof(t_fspaths));
	paths->bitmap_file = bitmap_path;
	paths->metadata_file = metadata_path;
	paths->blocks_folder = blocks_folder;
	paths->files_folder = files_folder;

	return paths;
}

void free_fspaths(t_fspaths* paths){
	free(paths->bitmap_file);
	free(paths->blocks_folder);
	free(paths->files_folder);
	free(paths->metadata_file);
	free(paths);
}

/*
 * Esta función chequea si existe el archivo Bitmap.bin,
 * en caso que no se asume que se debe crear la estructura
 * del filesystem desde cero y retorna el bitarray correspondiente.
*/
void init_fs(t_fspaths* fspaths){

	// TENGO QUE VERIFICAR QUE EXISTA LA CARPETA 'METADATA' O EL BITMAP.BIN?

	/*char* metadata_path = string_new();
	string_append_with_format(&metadata_path, "%s/Metadata", punto_montaje);

	DIR* metadata_dir = opendir(metadata_path);*/


	FILE* bitmap_file = fopen(fspaths->bitmap_file, "r");

	if(bitmap_file){
		// Caso metadata existente
		fclose(bitmap_file);
		printf("Bitmap.bin existente\n");

	} else {
		// Caso metadata NO existente
		printf("Archivo Bitmap.bin no encontrado, creando...\n");

		t_config* config_metadata = config_create(fspaths->metadata_file);
		int cantidad_bloques = config_get_int_value(config_metadata, "BLOCKS");
		create_bitmap(fspaths->bitmap_file, cantidad_bloques);

		create_blocks(fspaths->blocks_folder, cantidad_bloques);

		printf("Se crearon %d bloques\n", cantidad_bloques);
		config_destroy(config_metadata);
	}
}

void create_blocks(char* path, int cantidad){

	struct stat st = {0};
	if(stat(path, &st) == -1){
		mkdir(path, 0700);
	}

	FILE* block_file;

	for(int i = 0; i < cantidad; i++){
		char* block_path = string_new();
		string_append_with_format(&block_path, "%s/%d.bin", path, i);

		block_file = fopen(block_path, "wb");

		if(block_file == NULL){
			log_error(logger, "Error creando el bloque '%s'", block_path);
			abort();
		}

		fclose(block_file);

		free(block_path);
	}

}

void print_bitarray(t_bitarray* bitarray){
	size_t bitarray_size = bitarray_get_max_bit(bitarray);
	char* bits_string = string_new();
	for(int i = 0; i < bitarray_size; i++){
		bool bit = bitarray_test_bit(bitarray, i);
		if(bit){
			string_append(&bits_string, "1");
		} else {
			string_append(&bits_string, "0");
		}
	}

	printf("bitarray de largo %d: %s\n", bitarray_size, bits_string);
	free(bits_string);
}

t_bitarray* read_bitmap(char* bitmap_path){
	t_bitarray* bitarray = NULL;

	FILE* bitmap_ptr = fopen(bitmap_path, "rb");

	if(bitmap_ptr == NULL){
		char* mensaje_error = string_new();
		string_append_with_format(&mensaje_error, "No existe el archivo %s", bitmap_path);
		terminar_aplicacion(mensaje_error);
	}

	long file_size = get_file_size(bitmap_ptr);

	char* buffer = calloc(1, file_size);
	long bytes_read = fread(buffer, sizeof(char), file_size, bitmap_ptr);

	if(file_size == bytes_read){
		bitarray = bitarray_create_with_mode(buffer, file_size, LSB_FIRST);
		fclose(bitmap_ptr);
	} else {
		terminar_aplicacion("Error leyendo bitmap");
	}

	return bitarray;
}

t_bitarray* create_bitmap(char* file_path, int cantidad_bloques){
	t_bitarray* bitarray = NULL;

	FILE* bitmap_ptr = fopen(file_path, "wb");

	if(bitmap_ptr){
		int tam_en_bytes = cantidad_bloques/8;

		char* bits = calloc(tam_en_bytes, sizeof(char));

		fwrite(bits, tam_en_bytes, 1, bitmap_ptr);
		fclose(bitmap_ptr);

		bitarray = bitarray_create_with_mode(bits, tam_en_bytes, LSB_FIRST);
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

// Cada vez que seteo un bit reescribo todo el bitmap, se puede optimizar?
void set_bit(char* bitmap_path, int index, bool value){
	FILE* bitmap_file = fopen(bitmap_path, "rb+");

	long file_size = get_file_size(bitmap_file);
	char* data = malloc(file_size);
	int bytes_read = fread(data, sizeof(char), file_size, bitmap_file);
	if(file_size != bytes_read){
		terminar_aplicacion("error seteando bitmap");
	}

	t_bitarray* bitarray = bitarray_create_with_mode(data, file_size, LSB_FIRST);
	if(value){
		bitarray_set_bit(bitarray, index);
	} else {
		bitarray_clean_bit(bitarray, index);
	}

	// Reseteo el puntero al principio del archivo para reemplazar el contenido
	fseek(bitmap_file, 0, SEEK_SET);

	fwrite(data, sizeof(char), file_size, bitmap_file);
	fclose(bitmap_file);
	free(data);
	bitarray_destroy(bitarray);
}

bool existe_pokemon(char* nombre_pokemon){
	char* path_pokemon = string_new();
	char* lowered_pokemon = string_duplicate(nombre_pokemon);
	string_to_lower(lowered_pokemon);
	string_append_with_format(&path_pokemon, "%s/%s", fspaths->files_folder, lowered_pokemon);

	struct stat st = {0};
	bool pokemon_existe = stat(path_pokemon, &st) != -1;

	free(path_pokemon);
	free(lowered_pokemon);
	return pokemon_existe;
}

//Devuelve 0 si OK, -1 en caso de error
int crear_pokemon(char* nombre_pokemon){
	char* path_pokemon = string_new();
	char* lowered_pokemon = string_duplicate(nombre_pokemon);
	string_to_lower(lowered_pokemon);
	string_append_with_format(&path_pokemon, "%s/%s", fspaths->files_folder, lowered_pokemon);
	mkdir(path_pokemon , 0700);
	return 0;
}

int crear_metadata(char* path, bool is_file){
	FILE* metadata = fopen(path, "w");
	if(metadata){
		if(is_file){
			fprintf(metadata, "DIRECTORY=N\n"
					"SIZE=0\n"
					"BLOCKS=[]\n"
					"OPEN=N");
		} else {
			fprintf(metadata, "DIRECTORY=Y");
		}
		fclose(metadata);
		return 0;
	} else {
		return -1;
	}
}







