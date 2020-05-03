/*
 * conexiones.c
 *
 *  Created on: 2 mar. 2019
 *      Author: utnso
 */

#include "utils.h"

void* serializar_paquete(t_paquete* paquete, int bytes) {

	void* a_enviar = malloc(bytes);

	int offset = 0;

	memcpy(a_enviar + offset, &(paquete->codigo_operacion), sizeof(op_code));
	offset += sizeof(op_code);
	memcpy(a_enviar + offset, &(paquete->buffer->size), sizeof(int));
	offset += sizeof(int);
	memcpy(a_enviar + offset, paquete->buffer->stream, paquete->buffer->size);

	return a_enviar;
}

int crear_conexion(char *ip, char* puerto)
{
	struct addrinfo hints;
	struct addrinfo *server_info;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	getaddrinfo(ip, puerto, &hints, &server_info);

	int socket_cliente = socket(server_info->ai_family, server_info->ai_socktype, server_info->ai_protocol);

	if(connect(socket_cliente, server_info->ai_addr, server_info->ai_addrlen) == -1){
		perror("Error conectando al servidor");
	}

	freeaddrinfo(server_info);

	return socket_cliente;
}

void enviar_mensaje(char* mensaje, int socket_cliente) {

	t_paquete* paquete = malloc(sizeof(t_paquete));

	paquete->codigo_operacion = MENSAJE;
	t_buffer* buffer = malloc(sizeof(t_buffer));
	paquete->buffer = buffer;

	int mensaje_size = strlen(mensaje) + 1;

	buffer->size = mensaje_size;

	void* stream = malloc(buffer->size);

	memcpy(stream, mensaje, mensaje_size);

	buffer->stream = stream;

	int bytes = mensaje_size + 2 * sizeof(int);

	void* a_enviar = serializar_paquete(paquete, bytes);

	send(socket_cliente, a_enviar, bytes, 0);

	free_paquete(paquete);
	free(a_enviar);

}

char* recibir_mensaje(int socket_cliente){

	int codigo_operacion;
	recv(socket_cliente, (void*) &codigo_operacion, sizeof(int), MSG_WAITALL);

	if(codigo_operacion != MENSAJE){
		printf("El código %d no corresponde a un mensaje", codigo_operacion);
		return "";
	}

	int message_size;
	recv(socket_cliente, (void*) &message_size, sizeof(int), MSG_WAITALL);

	char* message = malloc(message_size);
	recv(socket_cliente, (void*) message, message_size, MSG_WAITALL);

	return message;

}

void free_paquete(t_paquete* paquete){
	free(paquete->buffer->stream);
	free(paquete->buffer);
	free(paquete);
}
