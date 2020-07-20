#include "utils/broker.h"


int main(){

	inicializar();

	int socket_servidor = iniciar_servidor_broker();

	pthread_create(&hilo_suscripciones, NULL, (void*) esperar_cliente, &socket_servidor);
	//pthread_create(&hilo_estado_queues,NULL,(void*) estado_de_queues, &socket_servidor);
	pthread_create(&hilo_mensajes, NULL, (void*) loop_productores, NULL);
	//pthread_create(&hilo_enviar_mensaje, NULL, (void*) mandar_mensajes, NULL);

	//pthread_join(hilo_estado_queues,NULL);
	pthread_join(hilo_suscripciones,NULL);
	pthread_join(hilo_mensajes,NULL);
	//pthread_join(hilo_enviar_mensaje,NULL);

	close(socket_servidor);
	//terminar_programa(logger,config);

	return 0;

}

void inicializar_colas(){
	inicializar_cola(&queue_new_pokemon, NEW_POKEMON);
	inicializar_cola(&queue_appeared_pokemon, APPEARED_POKEMON);
	inicializar_cola(&queue_catch_pokemon, CATCH_POKEMON);
	inicializar_cola(&queue_caught_pokemon, CAUGHT_POKEMON);
	inicializar_cola(&queue_get_pokemon, GET_POKEMON);
	inicializar_cola(&queue_localized_pokemon, LOCALIZED_POKEMON);
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
	(*nombre_cola)->cola = queue_create();
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

	pthread_mutex_lock(&(sem_cola[queue_a_mostrar->tipo_cola]));

	if (!queue_is_empty(queue_a_mostrar->cola)){

		t_info_mensaje* info = queue_peek(queue_a_mostrar->cola);

		uint32_t id_primero = info->id;
		uint32_t id_siguiente;

		do {
			info = queue_pop(queue_a_mostrar->cola);
			print_mensaje_de_cola(info);
			queue_push(queue_a_mostrar->cola,info);
			info = queue_peek(queue_a_mostrar->cola);
			id_siguiente = info->id;
		} while(id_primero != id_siguiente);

	}

	pthread_mutex_unlock(&(sem_cola[queue_a_mostrar->tipo_cola]));
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


void print_mensaje_de_cola(t_info_mensaje* mensaje){
	uint32_t id_mensaje = mensaje->id;
	printf("ID: %d\n",id_mensaje);
	queue_name id_cola = de_id_mensaje_a_cola(id_mensaje);
	void* msg = de_id_mensaje_a_mensaje(id_mensaje);
	t_buffer* mensaje_en_buffer = malloc(sizeof(t_buffer));
	mensaje_en_buffer->stream = msg;
	mensaje_en_buffer->size = de_id_mensaje_a_size(id_mensaje);
	void* msg_deserializado = deserializar_buffer(id_cola, mensaje_en_buffer);
	print_msg(id_cola, msg_deserializado);
	list_iterate(mensaje->a_quienes_fue_enviado, print_list_sockets_de_un_mensaje);
	list_iterate(mensaje->quienes_lo_recibieron, print_list_sockets_ACK_de_un_mensaje); // ACK
	free(mensaje_en_buffer->stream);
	free(mensaje_en_buffer);
}

void free_msg_cola(t_info_mensaje* mensaje){
	list_destroy_and_destroy_elements(mensaje->quienes_lo_recibieron,free);
	list_destroy_and_destroy_elements(mensaje->a_quienes_fue_enviado,free);
	free(mensaje);

}

void free_queue_msgs(t_cola_de_mensajes* cola_de_mensajes){
	queue_clean_and_destroy_elements(cola_de_mensajes->cola,free);
	list_destroy_and_destroy_elements(cola_de_mensajes->lista_suscriptores,free);
	free(cola_de_mensajes);
}

void sacar_de_cola(uint32_t id, int cola) {
	printf("Sacar_de_cola\n");

	pthread_mutex_lock(&sem_cola[cola]);
	printf("Entra al lock\n");
	t_cola_de_mensajes* queue = int_a_nombre_cola(cola);

	t_info_mensaje* mensaje = queue_peek(queue->cola);

	int control = 0;
	uint32_t id_primero = mensaje->id;
	uint32_t id_siguiente;
	printf("Entra al DO While\n");
	do {
		mensaje = queue_pop(queue->cola);

		if (mensaje->id == id) {
			control = 1;
			free_msg_cola(mensaje);
		} else {
			queue_push(queue->cola, mensaje);
			mensaje = queue_peek(queue->cola);
			id_siguiente = mensaje->id;
		}
		printf("control = %d\n",control);
		printf("id_primero= %d\n",id_primero);
		printf("Id_siguiente= %d\n",id_siguiente);
	} while (control == 0 && id_primero!= id_siguiente);

	printf("SALE DO While\n");
	pthread_mutex_unlock(&sem_cola[cola]);
	printf("Termina todo el Sacar COLA\n");
}

