#include "conexiones.h"

int iniciar_servidor(char* ip, char* puerto) {

	struct addrinfo hints;
	struct addrinfo *serverInfo;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	getaddrinfo(ip, puerto, &hints, &serverInfo);

	int server_socket = socket(serverInfo->ai_family, serverInfo->ai_socktype, serverInfo->ai_protocol);

	if (server_socket != -1) {
		int status_bind = bind(server_socket, serverInfo->ai_addr,
				serverInfo->ai_addrlen);
		if (status_bind != -1) {

			listen(server_socket, SOMAXCONN);
			freeaddrinfo(serverInfo);

			return server_socket;
		}
	}

	close(server_socket);
	perror("Error al asignar socket de escucha");

	return (-1);
}

int connect_sv(char* ip, char* puerto) {

	struct addrinfo hints;
	struct addrinfo *server_info;

	memset(&hints, 0, sizeof(hints));

	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	getaddrinfo(ip, puerto, &hints, &server_info);

	int socket_servidor = socket(server_info->ai_family, server_info->ai_socktype, server_info->ai_protocol);

	if (socket_servidor != -1) {
		int status_conexion = connect(socket_servidor, server_info->ai_addr,
				server_info->ai_addrlen);
		if (status_conexion != -1) {
			freeaddrinfo(server_info);
			return socket_servidor;
		}
	}

	freeaddrinfo(server_info);
	close(socket_servidor);
	return -1;

}

int conectar_como_productor(char* ip, char* puerto) {

	int socket_sv = connect_sv(ip, puerto);

	if (socket_sv == -1)
		return -1;

	queue_name cola_productor = PRODUCTOR;
	send(socket_sv, &cola_productor, sizeof(queue_name), 0);

	return socket_sv;
}

int suscribirse_a_cola(queue_name cola, char* ip, char* puerto) {

	int socket_servidor = connect_sv(ip, puerto);
	if(socket_servidor == -1) return -1;

	send(socket_servidor, &cola, sizeof(queue_name), 0);

	uint32_t respuesta_broker;
	if (recv(socket_servidor, &respuesta_broker, sizeof(uint32_t), MSG_WAITALL) <= 0) return -1;

	// Si la respuesta es 0, se conectó OK.
	if (respuesta_broker == 0) {
		return socket_servidor;
	} else {
		// Si la respuesta es 1, hubo un error.
		close(socket_servidor);
		return -1;
	}
}

uint32_t enviar_mensaje(char* ip, char* puerto, queue_name cola, void* estructura_mensaje, uint32_t id_mensaje, bool esperar_id_respuesta) {

	int socket_receptor = conectar_como_productor(ip, puerto);
	if (socket_receptor == -1) return -1; // Es un uint32_t y estoy devolviendo -1...

	t_paquete* paquete = malloc(sizeof(t_paquete));

	paquete->cola_msg = cola;
	paquete->buffer = malloc(sizeof(t_buffer));
	void* stream;
	int offset = 0;

	switch (cola) {
	case NEW_POKEMON:;
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

	case APPEARED_POKEMON:;
		appeared_pokemon_msg* msg_appeared = (appeared_pokemon_msg*) estructura_mensaje;

		paquete->buffer->size = sizeof(uint32_t) * 4 + msg_appeared->tamanio_nombre;
		stream = malloc(paquete->buffer->size);

		memcpy(stream + offset, &(msg_appeared->id_correlativo), sizeof(uint32_t));
		offset += sizeof(uint32_t);
		memcpy(stream + offset, &(msg_appeared->tamanio_nombre), sizeof(uint32_t));
		offset += sizeof(uint32_t);
		memcpy(stream + offset, msg_appeared->nombre_pokemon, msg_appeared->tamanio_nombre);
		offset += msg_appeared->tamanio_nombre;
		memcpy(stream + offset, &(msg_appeared->coordenada_X), sizeof(uint32_t));
		offset += sizeof(uint32_t);
		memcpy(stream + offset, &(msg_appeared->coordenada_Y),
				sizeof(uint32_t));

		break;

	case GET_POKEMON:;
		get_pokemon_msg* msg_get = (get_pokemon_msg*) estructura_mensaje;

		paquete->buffer->size = sizeof(uint32_t) + msg_get->tamanio_nombre;
		stream = malloc(paquete->buffer->size);

		memcpy(stream + offset, &(msg_get->tamanio_nombre), sizeof(uint32_t));
		offset += sizeof(uint32_t);
		memcpy(stream + offset, msg_get->nombre_pokemon, msg_get->tamanio_nombre);

		break;

	case LOCALIZED_POKEMON:;
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

	case CATCH_POKEMON:;
		catch_pokemon_msg* msg_catch = (catch_pokemon_msg*) estructura_mensaje;

		paquete->buffer->size = sizeof(uint32_t) * 3 + msg_catch->tamanio_nombre;
		stream = malloc(paquete->buffer->size);

		memcpy(stream + offset, &(msg_catch->tamanio_nombre), sizeof(uint32_t));
		offset += sizeof(uint32_t);
		memcpy(stream + offset, msg_catch->nombre_pokemon, msg_catch->tamanio_nombre);
		offset += msg_catch->tamanio_nombre;
		memcpy(stream + offset, &(msg_catch->coordenada_X), sizeof(uint32_t));
		offset += sizeof(uint32_t);
		memcpy(stream + offset, &(msg_catch->coordenada_Y), sizeof(uint32_t));

		break;

	case CAUGHT_POKEMON:;
		caught_pokemon_msg* msg_caught = (caught_pokemon_msg*) estructura_mensaje;

		paquete->buffer->size = sizeof(uint32_t) * 2;
		stream = malloc(paquete->buffer->size);

		memcpy(stream + offset, &(msg_caught->id_correlativo), sizeof(uint32_t));
		offset += sizeof(uint32_t);
		memcpy(stream + offset, &(msg_caught->resultado), sizeof(uint32_t));

		break;

	default:
		break;
	}

	// El tamaño total sería: stream (buffer_size) + id_cola (4) + buffer_size (4)
	int total_bytes = paquete->buffer->size + sizeof(queue_name) + sizeof(uint32_t);

	paquete->buffer->stream = stream;
	void* a_enviar = serializar_paquete(paquete, total_bytes);

	if (send(socket_receptor, a_enviar, total_bytes, 0) == -1) {
		perror("Error enviando mensaje");
		free_paquete(paquete);
		free(a_enviar);
		return -1;
	} else {
		if(id_mensaje > 0){
			send(socket_receptor, &id_mensaje, sizeof(uint32_t), 0);
		}
	}

	free_paquete(paquete);
	free(a_enviar);

	uint32_t id;
	if (esperar_id_respuesta) {
		int status_recv = recv(socket_receptor, &id, sizeof(uint32_t), MSG_WAITALL);
		if (status_recv <= 0) id = -1;
	} else {
		id = 0;
	}

	close(socket_receptor);
	return id;
}

void* serializar_paquete(t_paquete* paquete, int bytes) {

	void* a_enviar = malloc(bytes);
	int offset = 0;

	memcpy(a_enviar + offset, &(paquete->cola_msg), sizeof(queue_name));
	offset += sizeof(queue_name);
	memcpy(a_enviar + offset, &(paquete->buffer->size), sizeof(uint32_t));
	offset += sizeof(uint32_t);
	memcpy(a_enviar + offset, paquete->buffer->stream, bytes - sizeof(queue_name) - sizeof(uint32_t));

	return a_enviar;
}

void confirmar_recepcion(char* ip, char* puerto, queue_name cola, uint32_t id_mensaje, uint32_t mi_socket) {

	int socket_broker = conectar_como_productor(ip, puerto);

	t_paquete* paquete = malloc(sizeof(t_paquete));

	paquete->cola_msg = cola;
	paquete->buffer = malloc(sizeof(t_buffer));
	paquete->buffer->size = 0;
	paquete->buffer->stream = malloc(sizeof(uint32_t) * 2);

	int offset = 0;
	memcpy(paquete->buffer->stream, &id_mensaje, sizeof(uint32_t));
	offset += sizeof(uint32_t);
	memcpy(paquete->buffer->stream + offset, &mi_socket, sizeof(uint32_t));

	int bytes = sizeof(queue_name) + sizeof(uint32_t)*3;
	void* a_enviar = serializar_paquete(paquete, bytes);

	send(socket_broker, a_enviar, bytes, 0);
	close(socket_broker); // Chequear, seguramente no envie la info si lo cierro inmediatamente dsp del send.
}

void* recibir_mensaje(int socket, uint32_t* id, queue_name* tipo_msg, uint32_t* mi_socket) {

	t_paquete* paquete = malloc(sizeof(t_paquete));

	if (recv(socket, &(paquete->cola_msg), sizeof(uint32_t), MSG_WAITALL) <= 0) {
		free(paquete);
		return NULL;
	}
	*tipo_msg = paquete->cola_msg;

	paquete->buffer = malloc(sizeof(t_buffer));
	if (recv(socket, &(paquete->buffer->size), sizeof(uint32_t), MSG_WAITALL) <= 0) {
		free(paquete->buffer);
		free(paquete);
		return NULL;
	}

	paquete->buffer->stream = malloc(paquete->buffer->size);
	if (recv(socket, paquete->buffer->stream, paquete->buffer->size, MSG_WAITALL) <= 0) {
		free_paquete(paquete);
		return NULL;
	}

	if (recv(socket, id, sizeof(uint32_t), MSG_WAITALL) <= 0) {
		*id = 0; // No se envio o hubo un error recibiendo el ID.
	} else if(recv(socket, mi_socket, sizeof(uint32_t), MSG_WAITALL) <= 0) {
		*mi_socket = 0;
	}

	void* msg = deserializar_buffer(*tipo_msg, paquete->buffer, true);
	free_paquete(paquete);

	return msg;

}

void free_paquete(t_paquete* paquete) {
	free(paquete->buffer->stream);
	free(paquete->buffer);
	free(paquete);
}

void free_mensaje(queue_name tipo_msg, void* msg){
	switch(tipo_msg){
		case NEW_POKEMON:;
			new_pokemon_msg* new_pok = (new_pokemon_msg*) msg;
			free(new_pok->nombre_pokemon);
			break;

		case APPEARED_POKEMON:;
			appeared_pokemon_msg* appeared_pok = (appeared_pokemon_msg*) msg;
			free(appeared_pok->nombre_pokemon);
			break;
		case GET_POKEMON:;
			get_pokemon_msg* get_pok = (get_pokemon_msg*) msg;
			free(get_pok->nombre_pokemon);
			break;

		case LOCALIZED_POKEMON:;
			localized_pokemon_msg* localized_pok = (localized_pokemon_msg*) msg;
			free(localized_pok->nombre_pokemon);
			free(localized_pok->pares_coordenadas);
			break;

		case CATCH_POKEMON:;
			catch_pokemon_msg* catch_pok = (catch_pokemon_msg*) msg;
			free(catch_pok->nombre_pokemon);
			break;

		default:
			break;
		}

	free(msg);
}

// Retorna el tipo de msg que recibe por cola
void* deserializar_buffer(queue_name cola, void* buffer_ptr, bool incluir_correlativo) {

	t_buffer* buffer = (t_buffer*) buffer_ptr;

	uint32_t offset = 0;
	void* stream = buffer->stream;

	switch (cola) {

	case NEW_POKEMON:;
		new_pokemon_msg* msg_new = malloc(sizeof(new_pokemon_msg));

		memcpy(&(msg_new->tamanio_nombre), stream + offset, sizeof(uint32_t));
		offset += sizeof(uint32_t);
		msg_new->nombre_pokemon = malloc(msg_new->tamanio_nombre + 1);
		memcpy(msg_new->nombre_pokemon, stream + offset, msg_new->tamanio_nombre);
		msg_new->nombre_pokemon[msg_new->tamanio_nombre] = '\0';
		offset += msg_new->tamanio_nombre;
		memcpy(&(msg_new->coordenada_X), stream + offset, sizeof(uint32_t));
		offset += sizeof(uint32_t);
		memcpy(&(msg_new->coordenada_Y), stream + offset, sizeof(uint32_t));
		offset += sizeof(uint32_t);
		memcpy(&(msg_new->cantidad_pokemon), stream + offset, sizeof(uint32_t));

		return (void*) msg_new;

	case APPEARED_POKEMON:;
		appeared_pokemon_msg* msg_appeared = malloc(
				sizeof(appeared_pokemon_msg));

		if(incluir_correlativo){
			memcpy(&(msg_appeared->id_correlativo), stream + offset, sizeof(uint32_t));
			offset += sizeof(uint32_t);
		} else {
			msg_appeared->id_correlativo = 0;
		}
		memcpy(&(msg_appeared->tamanio_nombre), stream + offset, sizeof(uint32_t));
		offset += sizeof(uint32_t);
		msg_appeared->nombre_pokemon = malloc(msg_appeared->tamanio_nombre + 1);
		memcpy(msg_appeared->nombre_pokemon, stream + offset, msg_appeared->tamanio_nombre);
		msg_appeared->nombre_pokemon[msg_appeared->tamanio_nombre] = '\0';
		offset += msg_appeared->tamanio_nombre;
		memcpy(&(msg_appeared->coordenada_X), stream + offset, sizeof(uint32_t));
		offset += sizeof(uint32_t);
		memcpy(&(msg_appeared->coordenada_Y), stream + offset, sizeof(uint32_t));

		return (void*) msg_appeared;

	case GET_POKEMON:;
		get_pokemon_msg* msg_get = malloc(sizeof(get_pokemon_msg));

		memcpy(&(msg_get->tamanio_nombre), stream + offset, sizeof(uint32_t));
		offset += sizeof(uint32_t);
		msg_get->nombre_pokemon = malloc(msg_get->tamanio_nombre + 1);
		memcpy(msg_get->nombre_pokemon, stream + offset, msg_get->tamanio_nombre);
		msg_get->nombre_pokemon[msg_get->tamanio_nombre] = '\0';

		return (void*) msg_get;

	case LOCALIZED_POKEMON:;
		localized_pokemon_msg* msg_localized = malloc(sizeof(localized_pokemon_msg));

		if(incluir_correlativo){
			memcpy(&(msg_localized->id_correlativo), stream + offset, sizeof(uint32_t));
			offset += sizeof(uint32_t);
		} else {
			msg_localized->id_correlativo = 0;
		}
		memcpy(&(msg_localized->tamanio_nombre), stream + offset, sizeof(uint32_t));
		offset += sizeof(uint32_t);
		msg_localized->nombre_pokemon = malloc(msg_localized->tamanio_nombre + 1);
		memcpy(msg_localized->nombre_pokemon, stream + offset, msg_localized->tamanio_nombre);
		msg_localized->nombre_pokemon[msg_localized->tamanio_nombre] = '\0';
		offset += msg_localized->tamanio_nombre;
		memcpy(&(msg_localized->cantidad_posiciones), stream + offset, sizeof(uint32_t));
		offset += sizeof(uint32_t);

		uint32_t tamanio_pares_coordenadas = sizeof(uint32_t) * msg_localized->cantidad_posiciones * 2;

		msg_localized->pares_coordenadas = malloc(tamanio_pares_coordenadas);
		memcpy(msg_localized->pares_coordenadas, stream + offset, tamanio_pares_coordenadas);

		return (void*) msg_localized;

	case CATCH_POKEMON:;
		catch_pokemon_msg* msg_catch = malloc(sizeof(catch_pokemon_msg));

		memcpy(&(msg_catch->tamanio_nombre), stream + offset, sizeof(uint32_t));
		offset += sizeof(uint32_t);
		msg_catch->nombre_pokemon = malloc(msg_catch->tamanio_nombre + 1);
		memcpy(msg_catch->nombre_pokemon, stream + offset, msg_catch->tamanio_nombre);
		msg_catch->nombre_pokemon[msg_catch->tamanio_nombre] = '\0';
		offset += msg_catch->tamanio_nombre;
		memcpy(&(msg_catch->coordenada_X), stream + offset, sizeof(uint32_t));
		offset += sizeof(uint32_t);
		memcpy(&(msg_catch->coordenada_Y), stream + offset, sizeof(uint32_t));

		return (void*) msg_catch;

	case CAUGHT_POKEMON:;
		caught_pokemon_msg* msg_caught = malloc(sizeof(caught_pokemon_msg));

		if(incluir_correlativo){
			memcpy(&(msg_caught->id_correlativo), stream + offset, sizeof(uint32_t));
			offset += sizeof(uint32_t);
		} else {
			msg_caught->id_correlativo = 0;
		}
		memcpy(&(msg_caught->resultado), stream + offset, sizeof(uint32_t));

		return (void*) msg_caught;

	default:
		return NULL;
	}

}

void log_msg(queue_name cola, void* msg, t_log* logger){

}

void print_msg(queue_name cola, void* msg) {
	char* msg_string = msg_as_string(cola, msg);
	puts(msg_string);
	free(msg_string);
}

char* msg_as_string(queue_name cola, void* msg){
	char* string_msg = string_new();

	switch (cola) {
		case NEW_POKEMON:;
			new_pokemon_msg* msg_new = (new_pokemon_msg*) msg;
			string_append_with_format(&string_msg, "NEW_POKEMON {Nombre: %s, X: %d, Y: %d, Cantidad: %d}",
													msg_new->nombre_pokemon, msg_new->coordenada_X,
													msg_new->coordenada_Y, msg_new->cantidad_pokemon);
			break;

		case APPEARED_POKEMON:;
			appeared_pokemon_msg* msg_appeared = (appeared_pokemon_msg*) msg;
			string_append_with_format(&string_msg, "APPEARED_POKEMON {Nombre: %s, X: %d, Y: %d}", msg_appeared->nombre_pokemon,
													msg_appeared->coordenada_X, msg_appeared->coordenada_Y);
			break;

		case GET_POKEMON:;
			get_pokemon_msg* msg_get = (get_pokemon_msg*) msg;
			string_append_with_format(&string_msg, "GET_POKEMON {Nombre: %s}",
													msg_get->nombre_pokemon);
			break;

		case LOCALIZED_POKEMON:;
			localized_pokemon_msg* msg_localized = (localized_pokemon_msg*) msg;
			char* pares_coordenadas = string_new();

			if(msg_localized->cantidad_posiciones == 0){
				string_append(&pares_coordenadas, "n/a");
			} else {
				for(int i = 0; i < msg_localized->cantidad_posiciones*2; i++){
					if(i % 2 == 0){
						string_append_with_format(&pares_coordenadas, "(%d,", msg_localized->pares_coordenadas[i]);
					} else {
						string_append_with_format(&pares_coordenadas, "%d) ", msg_localized->pares_coordenadas[i]);
					}
				}
			}

			string_trim_right(&pares_coordenadas);
			string_append_with_format(&string_msg, "LOCALIZED_POKEMON {ID correlativo: %d, Nombre: %s, Cantidad posiciones: %d, Pares coordenadas: %s}",
													msg_localized->id_correlativo, msg_localized->nombre_pokemon,
													msg_localized->cantidad_posiciones, pares_coordenadas);
			free(pares_coordenadas);
			break;

		case CATCH_POKEMON:;
			catch_pokemon_msg* msg_catch = (catch_pokemon_msg*) msg;
			string_append_with_format(&string_msg, "CATCH_POKEMON {Nombre: %s, X: %d, Y: %d}",
													msg_catch->nombre_pokemon, msg_catch->coordenada_X, msg_catch->coordenada_Y);
			break;

		case CAUGHT_POKEMON:;
			caught_pokemon_msg* msg_caught = (caught_pokemon_msg*) msg;
			string_append_with_format(&string_msg, "CAUGHT_POKEMON {Id correlativo: %d, Resultado: %d}",
													msg_caught->id_correlativo, msg_caught->resultado);
			break;

		default:
			free(string_msg);
			return NULL;
	}

	return string_msg;
}

queue_name string_to_enum(char* string) {

	if (string_equals_ignore_case(string, "NEW_POKEMON")) {
		return NEW_POKEMON;
	} else if (string_equals_ignore_case(string, "APPEARED_POKEMON")) {
		return APPEARED_POKEMON;
	} else if (string_equals_ignore_case(string, "CATCH_POKEMON")) {
		return CATCH_POKEMON;
	} else if (string_equals_ignore_case(string, "CAUGHT_POKEMON")) {
		return CAUGHT_POKEMON;
	} else if (string_equals_ignore_case(string, "GET_POKEMON")) {
		return GET_POKEMON;
	} else if (string_equals_ignore_case(string, "LOCALIZED_POKEMON")) {
		return LOCALIZED_POKEMON;
	}
	return -1;
}

char* enum_to_string(queue_name cola) {

	switch (cola) {
	case NEW_POKEMON:
		return "NEW_POKEMON";
	case APPEARED_POKEMON:
		return "APPEARED_POKEMON";
	case GET_POKEMON:
		return "GET_POKEMON";
	case LOCALIZED_POKEMON:
		return "LOCALIZED_POKEMON";
	case CATCH_POKEMON:
		return "CATCH_POKEMON";
	case CAUGHT_POKEMON:
		return "CAUGHT_POKEMON";
	default:
		return NULL;
	}

}
// ------------------ Constructores

new_pokemon_msg* new_msg(char* nombre_pokemon, uint32_t x, uint32_t y, uint32_t cantidad) {
	new_pokemon_msg* msg = malloc(sizeof(new_pokemon_msg));

	msg->tamanio_nombre = strlen(nombre_pokemon);
	msg->nombre_pokemon = nombre_pokemon;
	msg->coordenada_X = x;
	msg->coordenada_Y = y;
	msg->cantidad_pokemon = cantidad;

	return msg;
}

appeared_pokemon_msg* appeared_msg(uint32_t id_correlativo, char* nombre_pokemon, uint32_t x, uint32_t y) {
	appeared_pokemon_msg* msg = malloc(sizeof(appeared_pokemon_msg));

	msg->id_correlativo = id_correlativo;
	msg->tamanio_nombre = strlen(nombre_pokemon);
	msg->nombre_pokemon = nombre_pokemon;
	msg->coordenada_X = x;
	msg->coordenada_Y = y;

	return msg;
}

get_pokemon_msg* get_msg(char* nombre_pokemon) {
	get_pokemon_msg* msg = malloc(sizeof(get_pokemon_msg));

	msg->tamanio_nombre = strlen(nombre_pokemon);
	msg->nombre_pokemon = nombre_pokemon;

	return msg;
}

localized_pokemon_msg* localized_msg(uint32_t id_corr, char* nombre_pokemon, uint32_t cant_posiciones, uint32_t* pares) {
	localized_pokemon_msg* msg = malloc(sizeof(localized_pokemon_msg));

	msg->id_correlativo = id_corr;
	msg->tamanio_nombre = strlen(nombre_pokemon);
	msg->nombre_pokemon = nombre_pokemon;
	msg->cantidad_posiciones = cant_posiciones;
	msg->pares_coordenadas = pares;

	return msg;
}

catch_pokemon_msg* catch_msg(char* nombre_pokemon, uint32_t x, uint32_t y) {
	catch_pokemon_msg* msg = malloc(sizeof(catch_pokemon_msg));

	msg->tamanio_nombre = strlen(nombre_pokemon);
	msg->nombre_pokemon = nombre_pokemon;
	msg->coordenada_X = x;
	msg->coordenada_Y = y;

	return msg;
}

caught_pokemon_msg* caught_msg(uint32_t id_corr, uint32_t resultado) {
	caught_pokemon_msg* msg = malloc(sizeof(caught_pokemon_msg));

	msg->id_correlativo = id_corr;
	msg->resultado = resultado;

	return msg;
}

// ------------------ Fin constructores
