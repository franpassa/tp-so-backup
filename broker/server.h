/*
 * server.h
 *
 *  Created on: 5 may. 2020
 *      Author: utnso
 */

#ifndef SERVER_H_
#define SERVER_H_



#include "broker.h"

void iniciar_servidor(char* ip,char* puerto){

		struct addrinfo hints;
		struct addrinfo *serverInfo;

		memset(&hints, 0, sizeof(hints));
		hints.ai_family = AF_UNSPEC;
		hints.ai_flags = AI_PASSIVE;
		hints.ai_socktype = SOCK_STREAM;

		getaddrinfo(ip, puerto, &hints, &serverInfo);

		int listeningSocket = socket(serverInfo->ai_family, serverInfo->ai_socktype, serverInfo->ai_protocol);

		bind(listeningSocket,serverInfo->ai_addr, serverInfo->ai_addrlen);
		freeaddrinfo(serverInfo);

		listen(listeningSocket, SOMAXCONN);

		printf("Estoy escuchando\n");


}


void esperar_cliente(int* socket_servidor){

	while(1){
		struct sockaddr_in dir_cliente;
		int tam_direccion = sizeof(struct sockaddr_in);

		int socket_cliente = accept(socket_servidor, (void*) &dir_cliente, &tam_direccion);
		int cola;
		if(recv(*socket_servidor, &cola, MSG_WAITALL) == -1) cola = -1;

		suscribir_a_cola(socket_cliente, cola);

		printf("El socket %d se suscribio a la cola %d\n", socket_cliente, cola);

	}

}

void suscribir_a_cola(int *socket_cliente, int cola) {	// *socket_cliente porque el hilo recibe punteros

	// Agregar Semaforo

	switch(cola){

	case 0:

		list_add(clientes_new_pokemon,socket_cliente);

		break;

	case 1:

		list_add(clientes_appeared_pokemon,socket_cliente);

		break;

	case 2:

		list_add(clientes_catch_pokemon,socket_cliente);

		break;

	case 3:

		list_add(clientes_caught_pokemon,socket_cliente);

		break;

	case 4:

		list_add(clientes_get_pokemon,socket_cliente);

		break;

	case 5:

		list_add(clientes_localized_pokemon,socket_cliente);

		break;

	default:

		break;

	}


}




#endif /* SERVER_H_ */
