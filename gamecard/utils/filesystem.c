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
	if(!metadata_config) terminar_aplicacion("No se encontro el Metadata del FS. Verificar punto de montaje.");
	FILE* bitmap_file = fopen(fspaths->bitmap_file, "r");

	if(bitmap_file){
		// Caso bitmap existente
		fclose(bitmap_file);
		log_info(logger, "Se detectó un FS existente");
		if(!files_folder_exists()) terminar_aplicacion("Metadata/Bitmap.bin existente pero no hay directorio 'Files'. Eliminar para inicializar un FS limpio.");
		inicializar_pokemons();

	} else {
		// Caso bitmap NO existente
		log_info(logger, "Inicializando FS...");

		int cantidad_bloques = config_get_int_value(metadata_config, "BLOCKS");
		if(create_bitmap(cantidad_bloques) == -1) terminar_aplicacion("Error creando bitmap");
		if(create_blocks(cantidad_bloques) == -1) terminar_aplicacion("Error creando bloques");
		if(files_folder_exists()) {
			eliminar_files();
		} else {
			create_files_folder();
		}

		log_info(logger, "Se crearon %d bloques\n", cantidad_bloques);
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

int reservar_bloque(){
	pthread_mutex_lock(&mutex_bitmap);
	int bitmap_fd = open(fspaths->bitmap_file, O_RDWR, S_IRUSR | S_IWUSR);
	struct stat sb;
	int block_index = -1;

	if(fstat(bitmap_fd, &sb) != -1){
		char* mapped_bitmap = mmap(NULL, sb.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, bitmap_fd, 0);

		t_bitarray* bitarray = bitarray_create_with_mode(mapped_bitmap, sb.st_size, LSB_FIRST);
		int bits = sb.st_size * 8;
		for(int i = 0; i < bits; i++){
			if(!bitarray_test_bit(bitarray, i)){
				bitarray_set_bit(bitarray, i);
				block_index = i;
				break;
			}
		}
		bitarray_destroy(bitarray);
		munmap(mapped_bitmap, sb.st_size);
		close(bitmap_fd);
	}
	pthread_mutex_unlock(&mutex_bitmap);

	return block_index;
}

void liberar_bloque(int bit_index){
	pthread_mutex_lock(&mutex_bitmap);
	int bitmap_fd = open(fspaths->bitmap_file, O_RDWR, S_IRUSR | S_IWUSR);
	struct stat sb;

	if(fstat(bitmap_fd, &sb) != -1){
		char* mapped_bitmap = mmap(NULL, sb.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, bitmap_fd, 0);

		t_bitarray* bitarray = bitarray_create_with_mode(mapped_bitmap, sb.st_size, LSB_FIRST);
		bitarray_clean_bit(bitarray, bit_index);
		bitarray_destroy(bitarray);
		munmap(mapped_bitmap, sb.st_size);
		close(bitmap_fd);
	}
	pthread_mutex_unlock(&mutex_bitmap);
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

void crear_metadata(char* nombre_pokemon, uint32_t file_size, t_list* blocks, bool keep_open){
	char* metadata_path = get_metadata_path(nombre_pokemon);
	char* blocks_string;
	if(blocks != NULL){
		blocks_string = list_to_string(blocks);
	} else {
		blocks_string = string_duplicate("[]");
	}
	FILE* metadata = fopen(metadata_path, "w");
	if(metadata == NULL) {
		free(metadata_path);
		return;
	}
	char open_flag = keep_open ? 'Y' : 'N';
	fprintf(metadata, "DIRECTORY=N\n"
					  "SIZE=%d\n"
					  "BLOCKS=%s\n"
					  "OPEN=%c", file_size, blocks_string, open_flag);

	free(blocks_string);
	free(metadata_path);
	fclose(metadata);
}

void actualizar_metadata(char* nombre_pokemon, uint32_t file_size, t_list* bloques, bool open_flag){
	char* metadata_path = get_metadata_path(nombre_pokemon);

	char* blocks_string = list_to_string(bloques);
	char* open_value;

	if(open_flag){
		open_value = string_duplicate("Y");
	} else {
		open_value = string_duplicate("N");
	}

	t_config* metadata = config_create(metadata_path);
	if(!metadata) terminar_aplicacion("METADATA DE ARCHIVO INEXISTENTE, CHEQUEAR CREACION");
	char* str_file_size = string_itoa(file_size);
	config_set_value(metadata, "SIZE", str_file_size);
	config_set_value(metadata, "BLOCKS", blocks_string);
	config_set_value(metadata, "OPEN", open_value);
	config_save(metadata);

	config_destroy(metadata);
	free(str_file_size);
	free(open_value);
	free(blocks_string);
	free(metadata_path);
}

int crear_file(char* nombre_pokemon, bool bloquear){
	char* path_pokemon = get_pokemon_path(nombre_pokemon);
	int status_creacion = mkdir(path_pokemon, 0700);
	crear_metadata(nombre_pokemon, 0, NULL, bloquear);
	free(path_pokemon);

	return status_creacion;
}

t_list* get_pokemon_blocks(char* nombre_pokemon){

	char* metadata_path = get_metadata_path(nombre_pokemon);
	t_config* pokemon_metadata = config_create(metadata_path);

	t_list* blocks_list = list_create();

	char** blocks_array = config_get_array_value(pokemon_metadata, "BLOCKS");
	char* block_str;
	int index = 0;

	while((block_str = blocks_array[index]) != NULL){
		int* block = malloc(sizeof(int));
		sscanf(block_str, "%d", block);
		list_add(blocks_list, block);
		index++;
	}

	free_array(blocks_array);
	config_destroy(pokemon_metadata);
	free(metadata_path);

	if(list_size(blocks_list) == 0){
		list_destroy(blocks_list);
		blocks_list = NULL;
	}

	return blocks_list;
}

int escribir_bloque(int bloque, char* contenido){
	char* block_path = get_block_path(bloque);
	FILE* block_file = fopen(block_path, "w");
	int bytes = 0;

	if(block_file != NULL){
		bytes = fwrite(contenido, sizeof(char), strlen(contenido), block_file);
		fclose(block_file);
	}
	free(block_path);

	return bytes;
}

int escribir_bloques(t_list* strings_por_bloque, t_list* blocks){
	int cant_bloques = list_size(blocks);
	int bytes = 0;

	for(int i = 0; i < cant_bloques; i++){
		int bloque = *(int*) list_get(blocks, i);
		char* contenido_bloque = (char*) list_get(strings_por_bloque, i);
		bytes += escribir_bloque(bloque, contenido_bloque);
	}
	return bytes;
}

t_list* obtener_strings_por_bloque(t_list* coordenadas){
	char* coordenadas_string = coordenadas_to_string(coordenadas);
	int block_size = config_get_int_value(metadata_config, "BLOCK_SIZE");
	t_list* strings_por_bloque = dividir_string_por_tamanio(coordenadas_string, block_size);

	free(coordenadas_string);

	return strings_por_bloque;
}

// El pokemon debe existir. Retorna el tamaño en bytes del file.
int agregar_posicion_pokemon(t_pokemon pokemon){
	t_list* bloques = get_pokemon_blocks(pokemon.nombre);
	char* contenido_bloques = get_blocks_content(bloques); // Se puede meter función intermedia para abstraer esta lógica
	t_list* coordenadas = string_to_coordenadas(contenido_bloques);
	free(contenido_bloques);
	add_coordenada(coordenadas, pokemon.posicion);
	obtener_bloques_necesarios(bloques, coordenadas);
	t_list* strings_por_bloque = obtener_strings_por_bloque(coordenadas);

	int bytes = escribir_bloques(strings_por_bloque, bloques);
	return bytes;
}

// Devuelve la cantidad de bytes escritos y actualiza la lista 'bloques' con los bloques utilizados.
int escribir_en_filesystem(t_list* bloques, t_list* coordenadas){
	int bytes_escritos;
	t_list* strings_por_bloque = NULL;

	obtener_bloques_necesarios(bloques, coordenadas);
	strings_por_bloque = obtener_strings_por_bloque(coordenadas);
	bytes_escritos = escribir_bloques(strings_por_bloque, bloques);

	list_destroy_and_destroy_elements(strings_por_bloque, free);

	return bytes_escritos;
}


int calcular_bloques_necesarios(t_list* lista_coordenadas){
	if(list_is_empty(lista_coordenadas)) return 0;

	int blocks_max_size = config_get_int_value(metadata_config, "BLOCK_SIZE");
	char* string_coordenadas = coordenadas_to_string(lista_coordenadas);
	int largo_coordenadas = string_length(string_coordenadas);

	div_t division = div(largo_coordenadas, blocks_max_size);

	free(string_coordenadas);

	return division.rem != 0 ? division.quot + 1 : division.quot;

}

int unlink_cb(const char *fpath, const struct stat *sb, int typeflag, struct FTW *ftwbuf){
    int rv = remove(fpath);
    if (rv) perror(fpath);

    return rv;
}

int rmrf(char *path){
    return nftw(path, unlink_cb, 64, FTW_DEPTH | FTW_PHYS);
}

void eliminar_file(char* nombre_file){
	char* file_path = string_duplicate(fspaths->files_folder);
	string_append_with_format(&file_path, "/%s", nombre_file);
	rmrf(file_path);
	free(file_path);
}

void eliminar_files(){
	struct dirent *dp;
	DIR *dir = opendir(fspaths->files_folder);
	if(!dir) return;

	while((dp = readdir(dir)) != NULL){
		if(!string_starts_with(dp->d_name, ".")){
			eliminar_file(dp->d_name);
		}
	}
	closedir(dir);
}

bool files_folder_exists(){
	char* path = fspaths->files_folder;
	struct stat st = {0};

	return stat(path, &st) != -1;
}

void create_files_folder(){
	char* path = fspaths->files_folder;
	mkdir(path, 0700);
}

// Leer contenido de los bloques a un string.
char* get_blocks_content(t_list* blocks){
	char* blocks_content = string_new();
	int blocks_qty = list_size(blocks);

	for(int i = 0; i < blocks_qty; i++){
		int block = *(int*) list_get(blocks, i);
		char* block_path = get_block_path(block);
		char* file_content = get_file_as_text(block_path);
		string_append(&blocks_content, file_content);
		free(file_content);
		free(block_path);
	}

	return blocks_content;
}

void obtener_bloques_necesarios(t_list* bloques_actuales, t_list* coordenadas){
	int bloques_necesarios = calcular_bloques_necesarios(coordenadas);
	int cant_bloques_actuales = list_size(bloques_actuales);

	if(cant_bloques_actuales < bloques_necesarios){
		int bloques_faltantes = bloques_necesarios - cant_bloques_actuales;
		for(int i = 0; i < bloques_faltantes; i++){
			int* nuevo_bloque = malloc(sizeof(int));
			*nuevo_bloque = reservar_bloque();
			list_add(bloques_actuales, nuevo_bloque);
		}
	} else if(bloques_necesarios < cant_bloques_actuales){
		int bloques_sobrantes = cant_bloques_actuales - bloques_necesarios;
		for(int i = 0; i < bloques_sobrantes; i++){
			int* bloque_sobrante = list_remove(bloques_actuales, list_size(bloques_actuales)-1);
			liberar_bloque(*bloque_sobrante);
			free(bloque_sobrante);
		}
	}
}

void inicializar_pokemons(){
	struct dirent *dp;
	DIR *dir = opendir(fspaths->files_folder);
	if(!dir) return;

	while((dp = readdir(dir)) != NULL){
		if(!string_starts_with(dp->d_name, ".")){
			inicializar_pokemon(dp->d_name, false);
			log_info(logger, "File %s existente en FS", dp->d_name);
		}
	}

	closedir(dir);

}
