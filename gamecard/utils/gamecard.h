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
#include <stdint.h>

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
	queue_name tipo_msg;
	void* msg;
	uint32_t id_msg;
} t_info_msg;


// -------- Globales --------

t_config* config;
t_config* metadata_config;
t_log* logger;
t_fspaths* fspaths;

// --- Hilos ---

pthread_t hilo_gameboy;
pthread_t hilo_invocador;
pthread_t hilo_new;
pthread_t hilo_catch;
pthread_t hilo_get;

// --- Semáforos ---

pthread_mutex_t mutex_bitmap;
pthread_mutex_t mutex_dict;
pthread_mutex_t mutex_reconexion;
pthread_mutex_t mutex_espera;
pthread_cond_t cond_reconectado;

// --- Diccionarios ---

t_dictionary* sem_files;

// --- Sockets suscripciones ---

int socket_new, socket_catch, socket_get;

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
t_list* dividir_string_por_tamanio(char* string, int tamanio);
int min(int a, int b);
void esperar_tiempo_retardo();
void free_array(char** array);
bool id_en_lista(t_list* lista, uint32_t id);
void agregar_id(t_list* lista, uint32_t id);
t_info_msg* init_info_msg(queue_name tipo, void* msg, uint32_t id);
void free_info_msg(t_info_msg* info);

// --- Filesystem ---

t_fspaths* init_fspaths(char* punto_montaje);
void init_fs();
int create_bitmap(int cantidad_bloques);
int create_blocks(int cantidad);
t_bitarray* read_bitmap(long* size_in_bytes);
void print_bitarray(t_bitarray* bitarray);
int reservar_bloque();
void liberar_bloque();
int get_bitmap_free_block();
char* get_block_path(int block);
void free_fspaths(t_fspaths* paths);
int write_blocks(char* contenido_bloques, t_list* blocks);
void crear_metadata(char* nombre_pokemon, uint32_t file_size, t_list* blocks, bool keep_open);
void actualizar_metadata(char* nombre_pokemon, uint32_t file_size, t_list* bloques, bool open_flag);
void eliminar_files();
char* get_blocks_content(t_list* blocks);
t_list* string_to_coordenadas(char* string_coordenadas);
int escribir_en_filesystem(t_list** bloques, t_list* coordenadas);
t_list* get_pokemon_blocks(char* nombre_pokemon);
int agregar_posicion_pokemon(t_pokemon pokemon);
void obtener_bloques_necesarios(t_list* bloques_actuales, t_list* coordenadas);
t_list* obtener_strings_por_bloque(t_list* coordenadas);
int escribir_bloques(t_list* strings_por_bloque, t_list* blocks);
int crear_file(char* nombre_pokemon, bool bloquear);
bool files_folder_exists();
void create_files_folder();
void inicializar_pokemons();

// --- Pokemons ---
t_coordenada init_coordenada(uint32_t x, uint32_t y, uint32_t cantidad);
t_pokemon init_pokemon(char* nombre, uint32_t x, uint32_t y, uint32_t cantidad);
// Devuelve un número de bloque si el pokemon existe, -1 si no
int obtener_ultimo_bloque(char* nombre_pokemon);
t_list* escribir_en_bloques(t_pokemon pokemon, t_list* lista_bloques, uint32_t *bytes_escritos);
pthread_mutex_t* inicializar_pokemon(char* nombre_pokemon, bool crear_file);
char* get_pokemon_path(char* nombre);
bool existe_pokemon(char* nombre_pokemon);
void toggle_open_flag(char* nombre_pokemon);
bool is_file_open(char* nombre_pokemon);
char* get_metadata_path(char* nombre_pokemon);
void new_pokemon(t_pokemon pokemon);
uint32_t catch_pokemon(t_pokemon pokemon);
uint32_t* get_pokemon(char* nombre_pokemon, uint32_t* cant_posiciones);

// --- Coordenadas ---
t_coordenada string_to_coordenada(char* string_coordenada);
t_list* string_to_coordenadas(char* string_coordenadas);
char* coordenadas_to_string(t_list* coordenadas);
void add_coordenada(t_list** lista_coordenadas, t_coordenada coordenada);
uint32_t* obtener_coordenadas(t_list* coordenadas, uint32_t* cant_coordenadas);
bool restar_coordenada(t_list* lista_coordenadas, t_coordenada coordenada);

// --- Comunicacion ---
int escuchar_gameboy();
void esperar_conexion(int socket_sv);
void manejar_msg_gameboy(int* socket_gameboy);
void procesar_msg(t_info_msg* info_msg);
void iniciar_hilos_escucha_broker();


#endif
