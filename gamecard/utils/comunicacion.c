#include "gamecard.h"

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

	procesar_msg(tipo_msg, msg);

}

void print_ints(int* elem){
	printf("%d\n", *elem);
}

void procesar_msg(queue_name tipo_msg, void* msg){
	char* msg_string = msg_as_string(tipo_msg, msg);
	if(msg_string) printf("msg recibido: %s\n", msg_string);

	switch(tipo_msg){
		case NEW_POKEMON:;
			new_pokemon_msg* new_pok = (new_pokemon_msg*) msg;
			t_pokemon pokemon = init_pokemon(new_pok->nombre_pokemon, new_pok->coordenada_X, new_pok->coordenada_Y, new_pok->cantidad_pokemon);
			if(!existe_pokemon(pokemon.nombre)){
				crear_pokemon(pokemon);
			} else {
				int ult_bloque = obtener_ultimo_bloque(pokemon.nombre);
				uint32_t bytes_escritos;
				t_list* nuevos_bloques = escribir_en_bloques(pokemon, ult_bloque, &bytes_escritos);
				list_iterate(nuevos_bloques, (void*) print_ints);
			}

			break;

		default:
			printf("Todavia no se maneja el tipo de msg %s\n", enum_to_string(tipo_msg));
	}
}
