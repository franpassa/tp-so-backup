#include "broker.h"


int iniciar_servidor() {

		struct addrinfo hints;
		struct addrinfo *serverInfo;

		char* ip = config_get_string_value(config,"IP_BROKER");
		char* puerto = config_get_string_value(config,"PUERTO_BROKER");

		//char* ip = "127.0.0.1";
		//char* puerto = "20002";

		memset(&hints, 0, sizeof(hints));
		hints.ai_family = AF_UNSPEC;
		hints.ai_socktype = SOCK_STREAM;
		hints.ai_flags = AI_PASSIVE;

		getaddrinfo(ip, puerto, &hints, &serverInfo);

		int listeningSocket = socket(serverInfo->ai_family, serverInfo->ai_socktype, serverInfo->ai_protocol);

		if(listeningSocket == -1){
			perror("Error al asignar socket de escucha\n");
			exit(1);
		}

		if(bind(listeningSocket,serverInfo->ai_addr, serverInfo->ai_addrlen) == -1){
			printf("Error en el bind, por favor cambiar puerto \n");
			exit(1);
		}


		listen(listeningSocket, SOMAXCONN);
		freeaddrinfo(serverInfo);

		printf("Servidor corriendo en %s:%s - SOCKET: %d\n", ip, puerto, listeningSocket);

		return listeningSocket;

}


void esperar_cliente(int* socket_servidor) { // Hilo esperar_cliente

	while(1){

		struct sockaddr_in dir_cliente;

		int tam_direccion = sizeof(struct sockaddr_in);

		int socket_cliente = accept(*socket_servidor, (void*) &dir_cliente, (socklen_t*) &tam_direccion);
		printf("Nuevo cliente entrante: %d\n", socket_cliente);

		queue_name cola;

		if(recv(socket_cliente, &cola, sizeof(queue_name), MSG_WAITALL) == -1) {
			perror("Error al recibir el mensaje:");
			printf("Error recibiendo mensaje\n");
			continue; // vuelve al loop

		}


		if(suscribir_a_cola(socket_cliente, cola) == -1){
			printf("%d es un codigo invalido\n", cola);
		} else {
			printf("el cliente %d se suscribio a la cola %s\n", socket_cliente, nombres_colas[cola]);
			int codigo_ok = 0;
			send(socket_cliente, &codigo_ok, sizeof(int), 0);
			//list iterate, printear contenido de lista
		}
	}

}

void print_list_chars(int* numero){
	printf("socket productor %d\n", *numero);
}

int suscribir_a_cola(int socket_cliente, queue_name cola) {

	int retorno = 0;

	pthread_mutex_lock(&semaforo_suscriber);

	switch(cola){

	case NEW_POKEMON:
		list_add(QUEUE_NEW_POKEMON->lista_suscriptores,&socket_cliente);
		break;

	case APPEARED_POKEMON:
		list_add(QUEUE_APPEARED_POKEMON->lista_suscriptores,&socket_cliente);
		break;

	case CATCH_POKEMON:
		list_add(QUEUE_CATCH_POKEMON->lista_suscriptores,&socket_cliente);
		break;

	case CAUGHT_POKEMON:
		list_add(QUEUE_CAUGHT_POKEMON->lista_suscriptores,&socket_cliente);
		break;

	case GET_POKEMON:
		list_add(QUEUE_GET_POKEMON->lista_suscriptores,&socket_cliente);
		break;

	case LOCALIZED_POKEMON:
		list_add(QUEUE_LOCALIZED_POKEMON->lista_suscriptores,&socket_cliente);
		break;

	case PRODUCTOR:
		pthread_mutex_lock(&mutex_productores);
		int* socket_productor = malloc(sizeof(int));
		*socket_productor = socket_cliente;
		list_add(sockets_productores, socket_productor);
		pthread_mutex_unlock(&mutex_productores);
		//list_iterate(sockets_productores, (void*) print_list_chars);
		break;


	default:
		// manejar error: codigo bien pero es invalido
		retorno = -1;
		break;

	}

	pthread_mutex_unlock(&semaforo_suscriber);

	return retorno;

}




