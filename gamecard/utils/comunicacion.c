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
	int* socket_gameboy = malloc(sizeof(int));
	*socket_gameboy = accept(socket_sv, (void*) &dir_cliente, (socklen_t*) &tam_direccion);

	pthread_t hilo_escucha;
	pthread_create(&hilo_escucha, NULL, (void*) manejar_msg_gameboy, socket_gameboy);
	pthread_detach(hilo_escucha);

}

void manejar_msg_gameboy(int* socket_gameboy){

	queue_name cola_productor;
	recv(*socket_gameboy, &cola_productor, sizeof(queue_name), MSG_WAITALL);

	uint32_t id_recibido;
	queue_name tipo_msg;
	void* msg = recibir_mensaje(*socket_gameboy, &id_recibido, &tipo_msg);
	free(socket_gameboy); // Se libera ya que se cierra dsp de enviar el msg.

	procesar_msg(tipo_msg, msg, id_recibido);
	free_mensaje(tipo_msg, msg);
}



void suscribir_a_colas(){
	char* ip_broker = config_get_string_value(config, "IP_BROKER");
	char* puerto_broker = config_get_string_value(config, "PUERTO_BROKER");
	int segundos_reintento = config_get_int_value(config, "TIEMPO_DE_REINTENTO_CONEXION");

	bool conexion_exitosa = false;

	while(!conexion_exitosa){
		socket_new = suscribirse_a_cola(NEW_POKEMON, ip_broker, puerto_broker);
		socket_catch = suscribirse_a_cola(CATCH_POKEMON, ip_broker, puerto_broker);
		socket_get = suscribirse_a_cola(GET_POKEMON, ip_broker, puerto_broker);

		if(socket_new != -1 && socket_catch != -1 && socket_get != -1){
			printf("Conexión con el Broker exitosa\n");
			conexion_exitosa = true;
		} else {
			printf("Falló la conexión con el Broker, reintentando en %d segundos...\n", segundos_reintento);
			sleep(segundos_reintento);
		}
	}
}

void escuchar_socket(int* socket){
	uint32_t id;
	queue_name cola;

	while(1){
		void* msg = recibir_mensaje(*socket, &id, &cola);
		if(msg != NULL){
			procesar_msg(cola, msg, id);
		} else {
			if(pthread_mutex_trylock(&mutex_reconexion) == 0){
				printf("Falló la conexión con el Broker, reintentando...\n");
				suscribir_a_colas();
				pthread_cond_broadcast(&cond_reconectado);
				pthread_mutex_unlock(&mutex_reconexion);
			} else {
				pthread_mutex_lock(&mutex_espera);
				pthread_cond_wait(&cond_reconectado, &mutex_espera);
				pthread_mutex_unlock(&mutex_espera);
			}
		}
	}
}

void iniciar_hilos_escucha_broker(){
	suscribir_a_colas();

	pthread_create(&hilo_new, NULL, (void*) escuchar_socket, &socket_new);
	pthread_detach(hilo_new);
	pthread_create(&hilo_catch, NULL, (void*) escuchar_socket, &socket_catch);
	pthread_detach(hilo_catch);
	pthread_create(&hilo_get, NULL, (void*) escuchar_socket, &socket_get);
	pthread_detach(hilo_get);
}

void procesar_msg(queue_name tipo_msg, void* msg, uint32_t id_msg){
	char* msg_string = msg_as_string(tipo_msg, msg);
	if(msg_string) printf("%s\n", msg_string);
	free(msg_string);

	t_pokemon pokemon;
	switch(tipo_msg){
		case NEW_POKEMON:;
			new_pokemon_msg* new_pok = (new_pokemon_msg*) msg;
			pokemon = init_pokemon(new_pok->nombre_pokemon, new_pok->coordenada_X, new_pok->coordenada_Y, new_pok->cantidad_pokemon);
			new_pokemon(pokemon);
			break;

		case CATCH_POKEMON:;
			catch_pokemon_msg* catch_pok = (catch_pokemon_msg*) msg;
			pokemon = init_pokemon(catch_pok->nombre_pokemon, catch_pok->coordenada_X, catch_pok->coordenada_Y, 1); // Lo inicializo con cantidad en uno ya que es lo que va a restar
			uint32_t resultado = catch_pokemon(pokemon);
			// caught_pokemon_msg* response_caught = caught_msg(id_msg, resultado);
			break;

		case GET_POKEMON:;
			get_pokemon_msg* get_pok = (get_pokemon_msg*) msg;
			pokemon = init_pokemon(get_pok->nombre_pokemon, 0, 0, 0);
			uint32_t cant_posiciones;
			uint32_t* posiciones = get_pokemon(pokemon.nombre, &cant_posiciones);
			localized_pokemon_msg* loc_msg = localized_msg(id_msg, get_pok->nombre_pokemon, cant_posiciones, posiciones);
			print_msg(LOCALIZED_POKEMON, (void*) loc_msg);
			break;

		default:
			printf("Todavia no se maneja el tipo de msg %s\n", enum_to_string(tipo_msg));
	}
}
