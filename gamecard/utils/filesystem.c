#include "gamecard.h"

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

void set_bit(int index, bool value){
	int bitmap_fd = open(fspaths->bitmap_file, O_RDWR, S_IRUSR | S_IWUSR);
	struct stat sb;

	if(fstat(bitmap_fd, &sb) != -1){
		char* mapped_bitmap = mmap(NULL, sb.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, bitmap_fd, 0);

		t_bitarray* bitarray = bitarray_create_with_mode(mapped_bitmap, sb.st_size, LSB_FIRST);
		if(value){
			bitarray_set_bit(bitarray, index); // Seteo o limpio el bit de interés
		} else {
			bitarray_clean_bit(bitarray, index);
		}
		bitarray_destroy(bitarray);
		munmap(mapped_bitmap, sb.st_size);
		close(bitmap_fd);
	}
}

int get_bitmap_free_block(){
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

void write_blocks(char* string, t_list* blocks, int block_max_size){
	int blocks_length = list_size(blocks);
	char* local_string = string_duplicate(string);
	char* string_inicial = local_string;

	for(int i = 0; i < blocks_length; i++){
		int block = *(int*) list_get(blocks, i);
		int cant_bytes = min(string_length(local_string), block_max_size);
		char* string_to_write = string_substring_until(local_string, cant_bytes);
		local_string = local_string + cant_bytes;


	}
	free(string_inicial);
}

char* format_pokemon(t_pokemon pokemon, uint32_t* length){
	char* string_pokemon = string_new();
	string_append_with_format(&string_pokemon, "%d-%d=%d", pokemon.posicion.x, pokemon.posicion.y, pokemon.posicion.cantidad);
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
t_list* escribir_en_filesystem(t_pokemon pokemon, t_list* lista_bloques, uint32_t *bytes_escritos){
	uint32_t bytes_linea;
	char* linea_input = format_pokemon(pokemon, &bytes_linea);
	int bloque_a_escribir;

	if(lista_bloques == NULL){
		pthread_mutex_lock(&mutex_bitmap);
		bloque_a_escribir = get_bitmap_free_block();
		pthread_mutex_unlock(&mutex_bitmap);
		if(bloque_a_escribir == -1) {
			printf("NO HAY MAS BLOQUES DISPONIBLES");
			return NULL;
		}
	} else {
		char* contenido_bloques = get_blocks_content(lista_bloques);
		t_list* lista_coordenadas = string_to_coordenadas(contenido_bloques);
		add_coordenada(lista_coordenadas, pokemon.posicion);
	}

}

/*void set_bit2(){
	*fd = open(path, O_RDONLY, S_IRUSR | S_IWUSR);
	struct stat sb;
	if(fstat(*fd, &sb) == -1) {
		perror("No se pudo abrir el archivo");
	} else {
		return mmap(NULL, sb.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, *fd, 0);
	}
}*/

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

// Leer contenido de los bloques a un string.
char* get_blocks_content(t_list* blocks){
	char* blocks_content = string_new();
	int blocks_qty = list_size(blocks);

	for(int i = 0; i < blocks_qty; i++){
		int block = *(int*) list_get(blocks, i); //Chequear si se puede desreferenciar así
		char* block_path = get_block_path(block);
		char* file_content = get_file_as_text(block_path);
		string_append(&blocks_content, file_content);
		free(file_content);
		free(block_path);
	}

	return blocks_content;
}
