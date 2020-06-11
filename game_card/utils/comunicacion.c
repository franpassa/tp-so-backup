#include "game_card.h"

int escuchar_gameboy(){
	char* ip = config_get_string_value(config, "IP_GAMECARD");
	char* puerto = config_get_string_value(config, "PUERTO_GAMECARD");

	int socket_sv = iniciar_servidor(ip, puerto);
	if(socket_sv == -1){
		terminar_aplicacion("Error iniciando el servidor");
	}

	while(1) esperar_conexion(socket_sv);

}

void esperar_conexion(int socket_sv){

	struct sockaddr_in dir_cliente;
	int tam_direccion = sizeof(struct sockaddr_in);
	int socket_gameboy = accept(socket_sv, (void*) &dir_cliente, (socklen_t*) &tam_direccion);

	pthread_t hilo_escucha;
	pthread_create(&hilo_escucha, NULL, (void*) manejar_msg_gameboy, &socket_gameboy);
	pthread_detach(hilo_escucha);

}

void manejar_msg_gameboy(int* socket_gameboy){

	queue_name cola_productor;
	recv(*socket_gameboy, &cola_productor, sizeof(queue_name), MSG_WAITALL);

	uint32_t id_recibido;
	queue_name tipo_msg;
	void* msg = recibir_mensaje(*socket_gameboy, &id_recibido, &tipo_msg);

	printf("msg de socket %d: %s\n", *socket_gameboy, msg_as_string(tipo_msg, msg));


}
