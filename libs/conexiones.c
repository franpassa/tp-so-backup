#include "conexiones.h"

int suscribirse_a_queue(queue_name cola, char* ip, char* puerto){

	struct addrinfo hints;
	struct addrinfo *server_info;

	memset(&hints, 0, sizeof(hints));

	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	getaddrinfo(ip, puerto, &hints, &server_info);

	int socket_servidor = socket(server_info->ai_family, server_info->ai_socktype, server_info->ai_protocol);

	int status_conexion = connect(socket_servidor, server_info->ai_addr, server_info->ai_addrlen);
	freeaddrinfo(server_info);

	if(status_conexion == -1) return status_conexion;

	send(socket_servidor, &cola, sizeof(queue_name), 0);

	int respuesta_broker;
	if(recv(socket_servidor, &respuesta_broker, sizeof(int), MSG_WAITALL) == -1) return -1;

	// Si la respuesta es 0, se conectó OK.
	if(respuesta_broker == 0) {
		return socket_servidor;
	} else {
		// Si la respuesta es -1, hubo un error en la suscripción.
		return respuesta_broker;
	}

}

void enviar_mensaje(queue_name cola, void* estructura_mensaje, int socket_receptor){

	t_paquete* paquete = malloc(sizeof(t_paquete));

	paquete->cola_msg = cola;
	paquete->buffer = malloc(sizeof(t_buffer));
	void* stream;
	int offset = 0;

	switch(cola){
		case NEW_POKEMON: ;
			new_pokemon_msg* msg_new = (new_pokemon_msg*) estructura_mensaje;

			paquete->buffer->size = sizeof(uint32_t) * 4 + msg_new->tamanio_nombre;
			stream = malloc(paquete->buffer->size);

			memcpy(stream + offset, &(msg_new->tamanio_nombre), sizeof(uint32_t));
			offset += sizeof(uint32_t);
			memcpy(stream + offset, msg_new->nombre_pokemon, msg_new->tamanio_nombre);
			offset += msg_new->tamanio_nombre;
			memcpy(stream + offset, &(msg_new->coordenada_X), sizeof(uint32_t));
			offset += sizeof(uint32_t);
			memcpy(stream + offset, &(msg_new->coordenada_Y), sizeof(uint32_t));
			offset += sizeof(uint32_t);
			memcpy(stream + offset, &(msg_new->cantidad_pokemon), sizeof(uint32_t));

			break;

		case APPEARED_POKEMON: ;
			appeared_pokemon_msg* msg_appeared = (appeared_pokemon_msg*) estructura_mensaje;

			paquete->buffer->size = sizeof(uint32_t) * 3 + msg_appeared->tamanio_nombre;
			stream = malloc(paquete->buffer->size);

			memcpy(stream + offset, &(msg_appeared->tamanio_nombre), sizeof(uint32_t));
			offset += sizeof(uint32_t);
			memcpy(stream + offset, msg_appeared->nombre_pokemon, msg_appeared->tamanio_nombre);
			offset += msg_appeared->tamanio_nombre;
			memcpy(stream + offset, &(msg_appeared->coordenada_X), sizeof(uint32_t));
			offset += sizeof(uint32_t);
			memcpy(stream + offset, &(msg_appeared->coordenada_Y), sizeof(uint32_t));

			break;

		case GET_POKEMON: ;
			get_pokemon_msg* msg_get = (get_pokemon_msg*) estructura_mensaje;

			paquete->buffer->size = sizeof(uint32_t) * 2 + msg_get->tamanio_nombre;
			stream = malloc(paquete->buffer->size);

			memcpy(stream + offset, &(msg_get->id_mensaje), sizeof(uint32_t));
			offset += sizeof(uint32_t);
			memcpy(stream + offset, &(msg_get->tamanio_nombre), sizeof(uint32_t));
			offset += sizeof(uint32_t);
			memcpy(stream + offset, msg_get->nombre_pokemon, msg_get->tamanio_nombre);

			break;

		case LOCALIZED_POKEMON: ;
			localized_pokemon_msg* msg_localized = (localized_pokemon_msg*) estructura_mensaje;

			uint32_t tamanio_pares_coordenadas = sizeof(uint32_t) * msg_localized->cantidad_posiciones * 2;

			paquete->buffer->size = sizeof(uint32_t) * 3 + tamanio_pares_coordenadas + msg_localized->tamanio_nombre;
			stream = malloc(paquete->buffer->size);

			memcpy(stream + offset, &(msg_localized->id_correlativo), sizeof(uint32_t));
			offset += sizeof(uint32_t);
			memcpy(stream + offset, &(msg_localized->tamanio_nombre), sizeof(uint32_t));
			offset += sizeof(uint32_t);
			memcpy(stream + offset, msg_localized->nombre_pokemon, msg_localized->tamanio_nombre);
			offset += msg_localized->tamanio_nombre;
			memcpy(stream + offset, &(msg_localized->cantidad_posiciones), sizeof(uint32_t));
			offset += sizeof(uint32_t);
			memcpy(stream + offset, msg_localized->pares_coordenadas, tamanio_pares_coordenadas);

			break;

		case CATCH_POKEMON: ;
			catch_pokemon_msg* msg_catch = (catch_pokemon_msg*) estructura_mensaje;

			paquete->buffer->size = sizeof(uint32_t) * 4 + msg_catch->tamanio_nombre;
			stream = malloc(paquete->buffer->size);

			memcpy(stream + offset, &(msg_catch->id_mensaje), sizeof(uint32_t));
			offset += sizeof(uint32_t);
			memcpy(stream + offset, &(msg_catch->tamanio_nombre), sizeof(uint32_t));
			offset += sizeof(uint32_t);
			memcpy(stream + offset, msg_catch->nombre_pokemon, msg_catch->tamanio_nombre);
			offset += msg_catch->tamanio_nombre;
			memcpy(stream + offset, &(msg_catch->coordenada_X), sizeof(uint32_t));
			offset += sizeof(uint32_t);
			memcpy(stream + offset, &(msg_catch->coordenada_Y), sizeof(uint32_t));

			break;

		case CAUGHT_POKEMON: ;
			caught_pokemon_msg* msg_caught = (caught_pokemon_msg*) estructura_mensaje;

			paquete->buffer->size = sizeof(uint32_t) * 2;
			stream = malloc(paquete->buffer->size);

			memcpy(stream + offset, &(msg_caught->id_correlativo), sizeof(uint32_t));
			offset += sizeof(uint32_t);
			memcpy(stream + offset, &(msg_caught->resultado), sizeof(uint32_t));

			break;
	}

	int total_bytes = paquete->buffer->size + sizeof(queue_name) + sizeof(uint32_t);

	paquete->buffer->stream = stream;
	void* a_enviar = serializar_paquete(paquete, total_bytes);

	send(socket_receptor, a_enviar, total_bytes, 0);

	free(paquete->buffer->stream);
	free(paquete->buffer);
	free(paquete);
	free(a_enviar);
}


void* serializar_paquete(t_paquete* paquete, int bytes){

	void* a_enviar = malloc(bytes);
	int offset = 0;

	memcpy(a_enviar + offset, &(paquete->cola_msg), sizeof(queue_name));
	offset += sizeof(queue_name);
	memcpy(a_enviar + offset, &(paquete->buffer->size), sizeof(uint32_t));
	offset += sizeof(uint32_t);
	memcpy(a_enviar + offset, paquete->buffer->stream, paquete->buffer->size);

	return a_enviar;
}
