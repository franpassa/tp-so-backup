#ifndef GAME_CARD_H_
#define GAME_CARD_H_


// -------- Librerias C estandar --------

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pthread.h>

// -------- Commons --------

#include <commons/log.h>
#include <commons/config.h>
#include <commons/string.h>
#include <commons/bitarray.h>
#include <commons/collections/list.h>

// -------- Cuarenteam libs --------

#include <conexiones.h>

// -------- Constantes --------

#define CONFIG_PATH "game_card.config"
#define LOG_PATH "game_card.log"

// -------- Structs --------

typedef struct {
	char* bitmap_file;
	char* metadata_file;
	char* blocks_folder;
	char* files_folder;
} t_fspaths;

typedef struct {
	char* nombre;
	uint32_t x;
	uint32_t y;
	uint32_t cantidad;
} t_pokemon;

typedef struct {
	char* nombre;
	char* contenido;
	t_list* bloques;
	uint32_t file_size;
} t_file;


// -------- Globales --------

t_config* config;
t_config* metadata_config;
t_log* logger;
t_fspaths* fspaths;

// -------- Funciones --------

// --- General ---

t_config* get_config(char* config_path);
t_log* crear_log(char* log_path);
void terminar_aplicacion(char* mensaje);
long get_file_size(FILE* file_ptr);
void agregar_a_lista(t_list* lista, int nuevo_elemento);
char* list_to_string(t_list* list);

// --- Filesystem ---

t_fspaths* init_fspaths(char* punto_montaje);
void init_fs();
int create_bitmap(int cantidad_bloques);
int create_blocks(int cantidad);
t_bitarray* read_bitmap(long* size_in_bytes);
void print_bitarray(t_bitarray* bitarray);
void set_bit(int index, bool value);
int get_free_block();
char* get_block_path(int block);
void free_fspaths(t_fspaths* paths);
char* write_block(char* linea, int block, int max_bytes, bool sobreescribir);
// En caso que sea un pokemon ya existente en el FS se le debe pasar el último bloque donde se guardó info, si es uno nuevo último bloque recibe un número negativo.
int crear_metadata(char* folder_path, uint32_t file_size, t_list* blocks);

// --- Pokemons ---
t_pokemon init_pokemon(char* nombre, uint32_t x, uint32_t y, uint32_t cantidad);
// Devuelve un número de bloque si el pokemon existe, -1 si no
int obtener_ultimo_bloque(char* nombre_pokemon);
t_list* escribir_en_bloques(t_pokemon pokemon, int ultimo_bloque, uint32_t *bytes_escritos);
int crear_pokemon(t_pokemon pokemon);
char* get_pokemon_path(char* nombre);
bool existe_pokemon(char* nombre_pokemon);

// --- Comunicacion ---
int escuchar_gameboy();
void esperar_conexion(int socket_sv);
void manejar_msg_gameboy(int* socket_gameboy);


#endif
