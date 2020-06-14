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
 * Chequea si existe el archivo Bitmap.bin,
 * en caso que no se asume que se debe crear la estructura
 * del filesystem desde cero y retorna el bitarray correspondiente.
*/
void init_fs(){

	metadata_config = config_create(fspaths->metadata_file);
	FILE* bitmap_file = fopen(fspaths->bitmap_file, "r");

	if(bitmap_file){
		// Caso bitmap existente
		fclose(bitmap_file);
		printf("Bitmap.bin existente\n");

	} else {
		// Caso bitmap NO existente
		printf("Archivo Bitmap.bin no encontrado, creando...\n");

		int cantidad_bloques = config_get_int_value(metadata_config, "BLOCKS");
		if(create_bitmap(cantidad_bloques) == -1) terminar_aplicacion("Error creando bitmap");
		if(create_blocks(cantidad_bloques) == -1) terminar_aplicacion("Error creando bloques");
		eliminar_files();

		printf("Se crearon %d bloques\n", cantidad_bloques);
	}
}

int create_blocks(int cantidad){
	if(cantidad <= 0) return -1;

	char* path = fspaths->blocks_folder;

	struct stat st = {0};
	if(stat(path, &st) == -1){
		mkdir(path, 0700);
	}

	FILE* block_file;
	for(int block_number = 0; block_number < cantidad; block_number++){
		char* block_path = string_new();
		string_append_with_format(&block_path, "%s/%d.bin", path, block_number);
		block_file = fopen(block_path, "w");

		if(block_file == NULL){
			free(block_path);
			return -1;
		}

		fclose(block_file);
		free(block_path);
	}

	return 0;
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

int create_bitmap(int cantidad_bloques){
	FILE* bitmap_ptr = fopen(fspaths->bitmap_file, "wb");

	if(bitmap_ptr == NULL) return -1;

	int tam_en_bytes = cantidad_bloques/8;
	char* bits = calloc(tam_en_bytes, sizeof(char));
	if(bits == NULL) {
		fclose(bitmap_ptr);
		return -1;
	}

	fwrite(bits, tam_en_bytes, 1, bitmap_ptr);
	fclose(bitmap_ptr);
	free(bits);

	return 0;
}

// Esta función es una obra de arte
void set_bit(int index, bool value){ // Revisar mmap
	FILE* bitmap_file = fopen(fspaths->bitmap_file, "rb+");

	div_t division = div(index, 8); // Divido el indice del bit por la cantidad de bits en un byte para obtener el byte y la posición del bit dentro de ese (cociente y resto).
	char* byte = calloc(1, sizeof(char));
	fseek(bitmap_file, division.quot, SEEK_SET); // Posiciono el puntero del bitmap a la posicion del byte que me interesa
	fread(byte, sizeof(char), 1, bitmap_file); // Leo ese byte
	t_bitarray* mini_bitarray = bitarray_create_with_mode(byte, sizeof(char), LSB_FIRST); // Creo un bitarray para poder modificar el bit
	if(value){
		bitarray_set_bit(mini_bitarray, division.rem); // Seteo o limpio el bit de interés
	} else {
		bitarray_clean_bit(mini_bitarray, division.rem);
	}
	fseek(bitmap_file, -1, SEEK_CUR); // Vuelvo un byte atrás para escribir el archivo
	fwrite(byte, sizeof(char), 1, bitmap_file); // Reescribo el byte con un bit modificado

	fclose(bitmap_file);
	free(byte);
	bitarray_destroy(mini_bitarray);
}

int get_free_block(){
	long file_size;
	t_bitarray* bitarray = read_bitmap(&file_size);
	long bits_in_file = file_size * 8;

	int free_block = -1;
	for(int block = 0; block < bits_in_file; block++){
		if(!bitarray_test_bit(bitarray, block)){
			free_block = block;
			break;
		}
	}
	free(bitarray->bitarray);
	free(bitarray);

	return free_block;
}

t_bitarray* read_bitmap(long* file_size){
	FILE* bitmap_file = fopen(fspaths->bitmap_file, "rb");
	if(bitmap_file == NULL) return NULL;

	*file_size = get_file_size(bitmap_file);
	char* data = malloc(*file_size);
	int bytes_read = fread(data, sizeof(char), *file_size, bitmap_file);

	if(*file_size != bytes_read) return NULL;

	t_bitarray* bitarray = bitarray_create_with_mode(data, *file_size, LSB_FIRST);
	fclose(bitmap_file);

	return bitarray;
}

long get_block_size(int block){
	char* block_path = get_block_path(block);

	FILE* block_file = fopen(block_path, "r");
	if(block_file == NULL) {
		free(block_path);
		return -1;
	}
	long block_size = get_file_size(block_file);
	fclose(block_file);
	free(block_path);

	return block_size;
}

char* get_block_path(int block){
	char* block_path = string_new();
	string_append_with_format(&block_path, "%s/%d.bin", fspaths->blocks_folder, block);

	return block_path;
}

// Retorno un puntero al string que no se pudo escribir (NULL si se escribió entero)
int write_block(char* string, int block, int max_bytes, bool sobreescribir){
	char* block_path = get_block_path(block);
	FILE* block_file;
	if(sobreescribir){
		block_file = fopen(block_path, "w");
	} else {
		block_file = fopen(block_path, "a");
	}
	free(block_path);

	bool line_jump = false;
	char* string_a_escribir;
	char* string_exceso = NULL;
	if(strlen(string) > max_bytes){
		string_a_escribir = string_substring_until(string, max_bytes);
		string_exceso = string_substring_from(string, max_bytes);
	} else {
		string_a_escribir = string_duplicate(string);
		if(strlen(string_a_escribir) < max_bytes) line_jump = true; // Si queda espacio para escribir en el bloque, agrego un salto de linea.
	}

	if(line_jump) string_append(&string_a_escribir, "\n");


	int bytes_escritos = fprintf(block_file, "%s\n", string_a_escribir);

	fclose(block_file);
	free(string_a_escribir);

	return bytes_escritos;
}

char* format_pokemon(t_pokemon pokemon, uint32_t* length){
	char* string_pokemon = string_new();
	string_append_with_format(&string_pokemon, "%d-%d=%d", pokemon.x, pokemon.y, pokemon.cantidad);
	*length = string_length(string_pokemon);

	return string_pokemon;
}

int crear_metadata(char* folder_path, uint32_t file_size, t_list* blocks){
	char* metadata_path = string_duplicate(folder_path);
	string_append(&metadata_path, "/Metadata.bin");
	char* blocks_string = list_to_string(blocks);
	FILE* metadata = fopen(metadata_path, "w");
	if(metadata == NULL) {
		free(metadata_path);
		return -1;
	}
	fprintf(metadata, "DIRECTORY=N\n"
					  "SIZE=%d\n"
					  "BLOCKS=[%s]\n"
					  "OPEN=N", file_size, blocks_string);

	free(metadata_path);
	fclose(metadata);
	return 0;
}

// Devuelvo una lista con los bloques escritos.
t_list* escribir_en_bloques(t_pokemon pokemon, int ultimo_bloque, uint32_t *bytes_escritos){
	bool nuevo_bloque = ultimo_bloque < 0;

	uint32_t bytes_linea;
	char* linea_input = format_pokemon(pokemon, &bytes_linea);
	int bloque_a_escribir;

	if(nuevo_bloque){
		bloque_a_escribir = get_free_block();
		if(bloque_a_escribir == -1) return NULL;
	} else {
		bloque_a_escribir = ultimo_bloque;
	}

	t_list* lista_bloques_escritos = list_create();
	int block_max_size = config_get_int_value(metadata_config, "BLOCK_SIZE");
	*bytes_escritos = 0;
	int bytes_x_operacion;
	char* linea_a_agregar = linea_input;
	while(1){
		bytes_x_operacion = write_block(linea_a_agregar, bloque_a_escribir, block_max_size, nuevo_bloque); // Devuelve los bytes que se escribieron.
		*bytes_escritos += bytes_x_operacion;
		set_bit(bloque_a_escribir, true); //  PUEDE SER QUE DE PROBLEMA DE SINCRONIZACION
		agregar_a_lista(lista_bloques_escritos, bloque_a_escribir);
		if(*bytes_escritos >= string_length(linea_input)) break;

		*bytes_escritos += bytes_linea - strlen(linea_a_agregar); // Los bytes que se escribieron son los totales menos lo que sobró
		bloque_a_escribir = get_free_block();
		nuevo_bloque = true;
	}
	free(linea_input);

	return lista_bloques_escritos;
}

int unlink_cb(const char *fpath, const struct stat *sb, int typeflag, struct FTW *ftwbuf){
    int rv = remove(fpath);
    if (rv) perror(fpath);

    return rv;
}

int rmrf(char *path){
    return nftw(path, unlink_cb, 64, FTW_DEPTH | FTW_PHYS);
}

void eliminar_files(){
	struct dirent *dp;
	DIR *dir = opendir(fspaths->files_folder);
	if(!dir) return;

	char* file_path;
	while((dp = readdir(dir)) != NULL){
		if(!string_starts_with(dp->d_name, ".")){
			file_path = string_duplicate(fspaths->files_folder);
			string_append_with_format(&file_path, "/%s", dp->d_name);
			rmrf(file_path);
			free(file_path);
		}
	}

	closedir(dir);
}
