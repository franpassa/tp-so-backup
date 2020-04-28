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

	if(status_conexion == -1){
		return status_conexion;
	}

	send(socket_servidor, &cola, sizeof(queue_name), 0);

	// Esperar mensaje de confirmación del Broker?

	return socket_servidor;

}

void enviar_mensaje(queue_name cola, void* estructura_mensaje, int socket_receptor){

	t_paquete* paquete = malloc(sizeof(t_paquete));
	int bytes;

	switch(cola){
		case NEW_POKEMON:
			new_pokemon_msg* msg = (new_pokemon_msg*) estructura_mensaje;
			break;

		case APPEARED_POKEMON:
			break;

		case GET_POKEMON:
			break;

		case LOCALIZED_POKEMON:
			break;

		case CATCH_POKEMON:
			break;

		case CAUGHT_POKEMON:
			break;

	void* a_enviar = serializar_paquete(paquete, bytes);
	send(socket_receptor, a_enviar, bytes, 0);

	}
}
