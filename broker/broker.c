#include "utils/broker.h"

int main(){

	inicializar();

	int socket_servidor = iniciar_servidor_broker();

	pthread_create(&hilo_suscripciones, NULL, (void*) esperar_cliente, &socket_servidor);
	pthread_create(&hilo_estado_queues,NULL,(void*) estado_de_queues, &socket_servidor); // no tiene espera activa
	pthread_create(&hilo_mensajes, NULL, (void*) loop_productores, NULL);
	pthread_create(&hilo_enviar_mensaje, NULL, (void*) mandar_mensajes, NULL);

	pthread_join(hilo_estado_queues,NULL);
	pthread_join(hilo_suscripciones,NULL);
	pthread_join(hilo_mensajes,NULL);
	pthread_join(hilo_enviar_mensaje,NULL);

	close(socket_servidor);

	return 0;

}

void inicializar_colas(){
	inicializar_cola(&queue_new_pokemon, NEW_POKEMON);
	inicializar_cola(&queue_appeared_pokemon, APPEARED_POKEMON);
	inicializar_cola(&queue_catch_pokemon, CATCH_POKEMON);
	inicializar_cola(&queue_caught_pokemon, CAUGHT_POKEMON);
	inicializar_cola(&queue_get_pokemon, GET_POKEMON);
	inicializar_cola(&queue_localized_pokemon, LOCALIZED_POKEMON);
	lista_de_particiones = list_create();
}

t_log* iniciar_logger(){
	t_log* mi_logger= log_create("log", "broker", 1, LOG_LEVEL_INFO);
	 if(mi_logger == NULL){
		 printf("Error inicializando logger");
		 exit(-1);
	 }

	 return mi_logger;
}


t_config* leer_config(){
	 t_config* mi_config = config_create("broker.config");
	 if(mi_config == NULL){
		 printf("Error inicializando config");
		 exit(-1);
	 }

	 return mi_config;

}

void terminar_programa(t_log* logger, t_config* config){
	log_destroy(logger);
	config_destroy(config);
}


void inicializar_cola(t_cola_de_mensajes** nombre_cola, queue_name id_cola){ // Esta bien inicializado?

	(*nombre_cola) = malloc(sizeof(t_cola_de_mensajes));
	(*nombre_cola)->lista_suscriptores = list_create();
	(*nombre_cola)->tipo_cola = id_cola;
}

t_cola_de_mensajes* int_a_nombre_cola(queue_name id){

	t_cola_de_mensajes* cola = NULL;

	switch (id){
		case NEW_POKEMON:
			cola = queue_new_pokemon;
			break;

		case APPEARED_POKEMON:
			cola = queue_appeared_pokemon;
			break;

		case CATCH_POKEMON:
			cola = queue_catch_pokemon;
			break;

		case CAUGHT_POKEMON:
			cola = queue_caught_pokemon;
			break;

		case GET_POKEMON:
			cola = queue_get_pokemon;
			break;

		case LOCALIZED_POKEMON:
			cola = queue_localized_pokemon;
			break;

		default:
			break;
}
	return cola;
}


void inicializar(){

	signal(SIGPIPE, SIG_IGN);
	signal(SIGUSR1, dump_de_cache);

	config = leer_config();
	logger = iniciar_logger();

	inicializar_colas();

	contador_id = 0;
	static const char* valores_colas[7] = {"NEW_POKEMON","APPEARED_POKEMON","CATCH_POKEMON","CAUGHT_POKEMON","GET_POKEMON","LOCALIZED_POKEMON", "PRODUCTOR"};
	memcpy(nombres_colas,valores_colas,sizeof(valores_colas));

	sockets_productores = list_create();
	pthread_mutex_init(&mutex_productores, NULL);
	pthread_mutex_init(&semaforo_id, NULL);
	pthread_mutex_init(&semaforo_struct_s, NULL);
	pthread_mutex_init(&semaforo_memoria, NULL);
	pthread_mutex_init(&sem_lru, NULL);
	sem_init(&semaforo_contador_productores, 0, 0);
	sem_init(&binario_mandar, 0, 0);

	for(int i = 0; i <= 5; i++){
		pthread_mutex_init(&(sem_cola[i]), NULL);
	}

	inicializar_memoria();

}

void estado_de_queues(){

	while(1){
		mostrar_estado_de_una_queue(queue_new_pokemon);
		mostrar_estado_de_una_queue(queue_appeared_pokemon);
		mostrar_estado_de_una_queue(queue_catch_pokemon);
		mostrar_estado_de_una_queue(queue_caught_pokemon);
		mostrar_estado_de_una_queue(queue_get_pokemon);
		mostrar_estado_de_una_queue(queue_localized_pokemon);
		sleep(4);
	}
}

void mostrar_estado_de_una_queue(t_cola_de_mensajes* cola){
	printf("%s\n", enum_to_string(cola->tipo_cola));
	printf("MENSAJES\n");
	recorrer_cola_de_mensajes_para_mostrar(cola);
	list_iterate(cola->lista_suscriptores, print_list_sockets);
	printf("\n");
}


void recorrer_cola_de_mensajes_para_mostrar(t_cola_de_mensajes* queue_a_mostrar){

	bool esCola(void* uno){
		t_struct_secundaria* particion =  (t_struct_secundaria*) uno;
		return particion->tipo_mensaje == queue_a_mostrar->tipo_cola;
	}
	t_list* lista_aux = list_filter(lista_de_particiones,esCola);

	pthread_mutex_lock(&(semaforo_struct_s));

	if (!list_is_empty(lista_aux)){

		for (int i = 0 ; i < list_size(lista_aux) ;i++){
			t_struct_secundaria* particion = list_get(lista_aux,i);
			print_mensaje_de_cola(particion);
		}
	}

	pthread_mutex_unlock(&(semaforo_struct_s));
}

void print_list_sockets(void* numero){
	printf("SUSCRIPTOR: %d\n", *(int*) numero);
}

void print_list_sockets_de_un_mensaje(void* numero){
	printf("ENVIADO A: %d\n", *(int*) numero);
}

void print_list_sockets_ACK_de_un_mensaje(void* numero){
	printf("CONFIRMADO POR: %d\n", *(int*) numero);
}


void print_mensaje_de_cola(t_struct_secundaria* particion){
	uint32_t id_mensaje = particion->id_mensaje;
	printf("ID: %d\n",id_mensaje);
	queue_name id_cola = particion->tipo_mensaje;
	void* msg = sacar_mensaje_de_memoria(particion->bit_inicio, particion->tamanio);
	t_buffer* mensaje_en_buffer = malloc(sizeof(t_buffer));
	void* msg_deserializado;
	if (id_cola == 1 || id_cola == 3 || id_cola == 5 ){

		void* stream_a_mandar = malloc(particion->tamanio + sizeof(uint32_t));
		memcpy(stream_a_mandar,&particion->id_correlativo,sizeof(uint32_t));
		memcpy(stream_a_mandar + sizeof(uint32_t), msg, particion->tamanio);
		mensaje_en_buffer->stream = stream_a_mandar;
		mensaje_en_buffer->size = particion->tamanio + sizeof(uint32_t);
		msg_deserializado = deserializar_buffer(id_cola, mensaje_en_buffer, true);
		print_msg(id_cola, msg_deserializado);
		free(stream_a_mandar);
	}else{
		mensaje_en_buffer->stream = msg;
		mensaje_en_buffer->size = particion->tamanio;
		msg_deserializado = deserializar_buffer(id_cola, mensaje_en_buffer, false);
		print_msg(id_cola, msg_deserializado);
	}
	free_mensaje(id_cola, msg_deserializado);
	list_iterate(particion->a_quienes_fue_enviado, print_list_sockets_de_un_mensaje);
	list_iterate(particion->quienes_lo_recibieron, print_list_sockets_ACK_de_un_mensaje); // ACK
	free(mensaje_en_buffer->stream);
	free(mensaje_en_buffer);

}

void free_queue_msgs(t_cola_de_mensajes* cola_de_mensajes){
	list_destroy_and_destroy_elements(cola_de_mensajes->lista_suscriptores, free);

	free(cola_de_mensajes);
}

