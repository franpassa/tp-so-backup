#include "conexiones.h"

int conectar_a_broker(queue_name cola, char* ip, char* puerto){

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
		// Si la respuesta es -1, hubo un error.
		return respuesta_broker;
	}
}

int enviar_mensaje(queue_name cola, void* estructura_mensaje, int socket_receptor){

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

	// El tamaño total sería: id_cola (4) + id_mensaje (4) + buffer_size (4) + stream (buffer_size)
	int total_bytes = paquete->buffer->size + sizeof(queue_name) + 2 * sizeof(uint32_t);

	paquete->buffer->stream = stream;
	void* a_enviar = serializar_paquete(paquete, total_bytes);

	send(socket_receptor, a_enviar, total_bytes, 0);

	free(paquete->buffer->stream);
	free(paquete->buffer);
	free(paquete);
	free(a_enviar);

	int id;
	recv(socket_receptor, &id, sizeof(int), MSG_WAITALL);

	return id;
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

void confirmar_recepcion(queue_name cola, uint32_t id_mensaje, int socket){
	t_paquete* paquete = malloc(sizeof(t_paquete));

	paquete->cola_msg = cola;
	paquete->buffer = malloc(sizeof(t_buffer));
	paquete->buffer->size = 0;
	paquete->buffer->stream = malloc(sizeof(uint32_t));

	memcpy(paquete->buffer->stream, &(id_mensaje), sizeof(uint32_t));
}

// Devuelve NULL si por alguna razón la cola del mensaje recibido es diferente a la recibida x parámetro
void* recibir_mensaje(queue_name cola, int socket){

	t_paquete* paquete = malloc(sizeof(t_paquete));

	recv(socket, &(paquete->cola_msg), sizeof(queue_name), MSG_WAITALL);

	if(cola != paquete->cola_msg) return NULL;

	paquete->buffer = malloc(sizeof(t_buffer));
	recv(socket, &(paquete->buffer->size), sizeof(queue_name), MSG_WAITALL);

	paquete->buffer->stream = malloc(paquete->buffer->size);
	recv(socket, paquete->buffer->stream, paquete->buffer->size, MSG_WAITALL);

	void* stream = paquete->buffer->stream;
	uint32_t offset = 0;

	switch(cola){

			case NEW_POKEMON: ;
				new_pokemon_msg* msg_new = malloc(sizeof(new_pokemon_msg));

				memcpy(&(msg_new->tamanio_nombre), stream + offset, sizeof(uint32_t));
				offset += sizeof(uint32_t);
				msg_new->nombre_pokemon = malloc(msg_new->tamanio_nombre);
				memcpy(msg_new->nombre_pokemon, stream + offset, msg_new->tamanio_nombre);
				offset += msg_new->tamanio_nombre;
				memcpy(&(msg_new->coordenada_X), stream + offset, sizeof(uint32_t));
				offset += sizeof(uint32_t);
				memcpy(&(msg_new->coordenada_Y), stream + offset, sizeof(uint32_t));
				offset += sizeof(uint32_t);
				memcpy(&(msg_new->cantidad_pokemon), stream + offset, sizeof(uint32_t));

				free_paquete(paquete);

				return (void*) msg_new;

			case APPEARED_POKEMON: ;
				appeared_pokemon_msg* msg_appeared = malloc(sizeof(appeared_pokemon_msg));

				memcpy(&(msg_appeared->tamanio_nombre), stream + offset, sizeof(uint32_t));
				offset += sizeof(uint32_t);
				msg_appeared->nombre_pokemon = malloc(msg_appeared->tamanio_nombre);
				memcpy(msg_appeared->nombre_pokemon, stream + offset, msg_appeared->tamanio_nombre);
				offset += msg_appeared->tamanio_nombre;
				memcpy(&(msg_appeared->coordenada_X), stream + offset, sizeof(uint32_t));
				offset += sizeof(uint32_t);
				memcpy(&(msg_appeared->coordenada_Y), stream + offset, sizeof(uint32_t));

				free_paquete(paquete);

				return (void*) msg_appeared;

			case GET_POKEMON: ;
				get_pokemon_msg* msg_get = malloc(sizeof(get_pokemon_msg));

				memcpy(&(msg_get->tamanio_nombre), stream + offset, sizeof(uint32_t));
				offset += sizeof(uint32_t);
				msg_get->nombre_pokemon = malloc(msg_get->tamanio_nombre);
				memcpy(msg_get->nombre_pokemon, stream + offset, msg_get->tamanio_nombre);

				free_paquete(paquete);

				return (void*) msg_get;

			case LOCALIZED_POKEMON: ;
				localized_pokemon_msg* msg_localized = malloc(sizeof(localized_pokemon_msg));

				memcpy(&(msg_localized->id_correlativo), stream + offset, sizeof(uint32_t));
				offset += sizeof(uint32_t);
				memcpy(&(msg_localized->tamanio_nombre), stream + offset, sizeof(uint32_t));
				offset += sizeof(uint32_t);
				msg_localized->nombre_pokemon = malloc(msg_localized->tamanio_nombre);
				memcpy(msg_localized->nombre_pokemon, stream + offset, msg_localized->tamanio_nombre);
				offset += msg_localized->tamanio_nombre;
				memcpy(&(msg_localized->cantidad_posiciones), stream + offset, sizeof(uint32_t));
				offset += sizeof(uint32_t);

				uint32_t tamanio_pares_coordenadas = sizeof(uint32_t) * msg_localized->cantidad_posiciones * 2;

				msg_localized->pares_coordenadas = malloc(tamanio_pares_coordenadas);
				memcpy(msg_localized->pares_coordenadas, stream + offset, tamanio_pares_coordenadas);

				free_paquete(paquete);

				return (void*) msg_localized;

			case CATCH_POKEMON: ;
				catch_pokemon_msg* msg_catch = malloc(sizeof(catch_pokemon_msg));

				memcpy(&(msg_catch->tamanio_nombre), stream + offset, sizeof(uint32_t));
				offset += sizeof(uint32_t);
				msg_catch->nombre_pokemon = malloc(msg_catch->tamanio_nombre);
				memcpy(msg_catch->nombre_pokemon, stream + offset, msg_catch->tamanio_nombre);
				offset += msg_catch->tamanio_nombre;
				memcpy(&(msg_catch->coordenada_X), stream + offset, sizeof(uint32_t));
				offset += sizeof(uint32_t);
				memcpy(&(msg_catch->coordenada_Y), stream + offset, sizeof(uint32_t));

				free_paquete(paquete);

				return (void*) msg_catch;

			case CAUGHT_POKEMON: ;
				caught_pokemon_msg* msg_caught = malloc(sizeof(caught_pokemon_msg));

				memcpy(&(msg_caught->id_correlativo), stream + offset, sizeof(uint32_t));
				offset += sizeof(uint32_t);
				memcpy(&(msg_caught->resultado), stream + offset, sizeof(uint32_t));

				free_paquete(paquete);

				return (void*) msg_caught;
		}

	return NULL;

}

void free_paquete(t_paquete* paquete){
	free(paquete->buffer->stream);
	free(paquete->buffer);
	free(paquete);
}

// ------------------ Constructores

new_pokemon_msg* new_msg(char* nombre_pokemon, uint32_t x, uint32_t y, uint32_t cantidad){
	new_pokemon_msg* msg = malloc(sizeof(new_pokemon_msg));

	msg->tamanio_nombre = strlen(nombre_pokemon) + 1;
	msg->nombre_pokemon = nombre_pokemon;
	msg->coordenada_X = x;
	msg->coordenada_Y = y;
	msg->cantidad_pokemon = cantidad;

	return msg;
}

appeared_pokemon_msg* appeared_msg(char* nombre_pokemon, uint32_t x, uint32_t y){
	appeared_pokemon_msg* msg = malloc(sizeof(appeared_pokemon_msg));

	msg->tamanio_nombre = strlen(nombre_pokemon) + 1;
	msg->nombre_pokemon = nombre_pokemon;
	msg->coordenada_X = x;
	msg->coordenada_Y = y;

	return msg;
}

get_pokemon_msg* get_msg(char* nombre_pokemon){
	get_pokemon_msg* msg = malloc(sizeof(get_pokemon_msg));

	msg->tamanio_nombre = strlen(nombre_pokemon) + 1;
	msg->nombre_pokemon = nombre_pokemon;

	return msg;
}

localized_pokemon_msg* localized_msg(uint32_t id_corr, char* nombre_pokemon, uint32_t cant_posiciones, uint32_t* pares){
	localized_pokemon_msg* msg = malloc(sizeof(localized_pokemon_msg));

	msg->id_correlativo = id_corr;
	msg->tamanio_nombre = strlen(nombre_pokemon) + 1;
	msg->nombre_pokemon = nombre_pokemon;
	msg->cantidad_posiciones = cant_posiciones;
	msg->pares_coordenadas = pares;

	return msg;
}

catch_pokemon_msg* catch_msg(char* nombre_pokemon, uint32_t x, uint32_t y){
	catch_pokemon_msg* msg = malloc(sizeof(catch_pokemon_msg));

	msg->tamanio_nombre = strlen(nombre_pokemon) + 1;
	msg->nombre_pokemon = nombre_pokemon;
	msg->coordenada_X = x;
	msg->coordenada_Y = y;

	return msg;
}

caught_pokemon_msg* caught_msg(uint32_t id_corr, uint32_t resultado){
	caught_pokemon_msg* msg = malloc(sizeof(caught_pokemon_msg));

	msg->id_correlativo = id_corr;
	msg->resultado = resultado;

	return msg;
}

// ------------------ Fin constructores
