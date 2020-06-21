#ifndef GAME_CARD_H_
#define GAME_CARD_H_

#define _XOPEN_SOURCE 500

// -------- Librerias C estandar --------

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pthread.h>
#include <ftw.h>
#include <dirent.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <math.h>

// -------- Commons --------

#include <commons/log.h>
#include <commons/config.h>
#include <commons/string.h>
#include <commons/bitarray.h>
#include <commons/collections/list.h>

// -------- Cuarenteam libs --------

#include <conexiones.h>

// -------- Constantes --------

#define CONFIG_PATH "gamecard.config"
#define LOG_PATH "gamecard.log"

// -------- Structs --------

typedef struct {
	char* bitmap_file;
	char* metadata_file;
	char* blocks_folder;
	char* files_folder;
} t_fspaths;

typedef struct {
	uint32_t x;
	uint32_t y;
	uint32_t cantidad;
} t_coordenada;

typedef struct {
	char* nombre;
	t_coordenada posicion;
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

// --- Hilos ---

pthread_t hilo_gameboy;

// --- Semáforos ---

pthread_mutex_t mutex_bitmap;

// -------- Funciones --------

// --- General ---

t_config* get_config(char* config_path);
t_log* crear_log(char* log_path);
void terminar_aplicacion(char* mensaje);
long get_file_size(FILE* file_ptr);
void agregar_a_lista(t_list* lista, int nuevo_elemento);
char* list_to_string(t_list* list);
char* get_last(char** array);
char* get_file_as_text(char* file_path);
int min(int a, int b);
t_list* dividir_string_por_tamanio(char* string, int tamanio);

// --- Filesystem ---

t_fspaths* init_fspaths(char* punto_montaje);
void init_fs();
int create_bitmap(int cantidad_bloques);
int create_blocks(int cantidad);
t_bitarray* read_bitmap(long* size_in_bytes);
void print_bitarray(t_bitarray* bitarray);
void set_bit(int index, bool value);
int get_bitmap_free_block();
char* get_block_path(int block);
void free_fspaths(t_fspaths* paths);
void write_blocks(char* string, t_list* blocks, int block_max_size);
// En caso que sea un pokemon ya existente en el FS se le debe pasar el último bloque donde se guardó info, si es uno nuevo último bloque recibe un número negativo.
int crear_metadata(char* folder_path, uint32_t file_size, t_list* blocks);
void eliminar_files();
char* get_blocks_content(t_list* blocks);
t_list* string_to_coordenadas(char* string_coordenadas);
t_list* escribir_en_filesystem(t_pokemon pokemon, t_list* lista_bloques, uint32_t *bytes_escritos);
//t_list* get_pokemon_blocks(char* nombre_pokemon);

// --- Pokemons ---
t_coordenada init_coordenada(uint32_t x, uint32_t y, uint32_t cantidad);
t_pokemon init_pokemon(char* nombre, uint32_t x, uint32_t y, uint32_t cantidad);
// Devuelve un número de bloque si el pokemon existe, -1 si no
int obtener_ultimo_bloque(char* nombre_pokemon);
t_list* escribir_en_bloques(t_pokemon pokemon, t_list* lista_bloques, uint32_t *bytes_escritos);
int crear_pokemon(t_pokemon pokemon);
char* get_pokemon_path(char* nombre);
bool existe_pokemon(char* nombre_pokemon);
void toggle_open_flag(char* nombre_pokemon);
bool is_file_open(char* nombre_pokemon);

// --- Coordenadas ---
t_coordenada string_to_coordenada(char* string_coordenada);
t_list* string_to_coordenadas(char* string_coordenadas);
char* coordenadas_to_string(t_list* coordenadas);
void add_coordenada(t_list* lista_coordenadas, t_coordenada coordenada);
t_coordenada* find_coordenada(t_list* lista_coordenadas, t_coordenada coordenada);

// --- Comunicacion ---
int escuchar_gameboy();
void esperar_conexion(int socket_sv);
void manejar_msg_gameboy(int* socket_gameboy);
void procesar_msg(queue_name tipo_msg, void* msg);


#endif
