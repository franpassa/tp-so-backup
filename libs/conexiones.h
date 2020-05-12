#ifndef CONEXIONES_H_
#define CONEXIONES_H_

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <string.h>
#include <stdint.h>

// Tipos mensajes

typedef enum {
	NEW_POKEMON,
	APPEARED_POKEMON,
	CATCH_POKEMON,
	CAUGHT_POKEMON,
	GET_POKEMON,
	LOCALIZED_POKEMON,
	PRODUCTOR // Este tipo de mensaje se utiliza para cuando los módulos se conectan para enviar mensajes.
} queue_name;

// Estructuras mensajes

typedef struct {
	uint32_t tamanio_nombre;
	char* nombre_pokemon;
	uint32_t coordenada_X;
	uint32_t coordenada_Y;
	uint32_t cantidad_pokemon;
} new_pokemon_msg;

typedef struct {
	uint32_t tamanio_nombre;
	char* nombre_pokemon;
	uint32_t coordenada_X;
	uint32_t coordenada_Y;
} appeared_pokemon_msg;

typedef struct {
	uint32_t tamanio_nombre;
	char* nombre_pokemon;
} get_pokemon_msg;

typedef struct {
	uint32_t id_correlativo;
	uint32_t tamanio_nombre;
	char* nombre_pokemon;
	uint32_t cantidad_posiciones;
	uint32_t* pares_coordenadas;
} localized_pokemon_msg;

typedef struct {
	uint32_t tamanio_nombre;
	char* nombre_pokemon;
	uint32_t coordenada_X;
	uint32_t coordenada_Y;
} catch_pokemon_msg;

typedef struct {
	uint32_t id_correlativo;
	uint32_t resultado;
} caught_pokemon_msg;

// Serializacion

typedef struct {
	uint32_t size;
	void* stream;
} t_buffer;

typedef struct {
	queue_name cola_msg;
	t_buffer* buffer;
} t_paquete;

new_pokemon_msg* new_msg(char* nombre_pokemon, uint32_t x, uint32_t y, uint32_t cantidad);
appeared_pokemon_msg* appeared_msg(char* nombre_pokemon, uint32_t x, uint32_t y);
get_pokemon_msg* get_msg(char* nombre_pokemon);
localized_pokemon_msg* localized_msg(uint32_t id_corr, char* nombre_pokemon, uint32_t cant_posiciones, uint32_t* pares);
catch_pokemon_msg* catch_msg(char* nombre_pokemon, uint32_t x, uint32_t y);
caught_pokemon_msg* caught_msg(uint32_t id_corr, uint32_t resultado);

int conectar_a_broker(queue_name cola, char* ip, char* puerto);
int enviar_mensaje(queue_name tipo_msg, void* mensaje, int socket);
void* recibir_mensaje(queue_name cola, int socket);
void confirmar_recepcion(queue_name cola, uint32_t id_mensaje, int socket);
void* serializar_paquete(t_paquete* paquete, int bytes);
void free_paquete(t_paquete* paquete);

#endif
