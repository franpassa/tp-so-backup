#include "utils/broker.h"


int main(){

	inicializar();

	int socket_servidor = iniciar_servidor_broker();

	pthread_create(&hilo_suscripciones, NULL, (void*) esperar_cliente, &socket_servidor);
	pthread_create(&hilo_estado_queues,NULL,(void*) estado_de_queues,&socket_servidor);
	pthread_create(&hilo_mensajes, NULL, (void*) loop_productores, NULL);
	pthread_create(&hilo_enviar_mensaje, NULL, (void*) mandar_mensajes, NULL);

	pthread_join(hilo_estado_queues,NULL);
	pthread_join(hilo_suscripciones,NULL);
	pthread_join(hilo_mensajes,NULL);
	pthread_join(hilo_enviar_mensaje, NULL);

	close(socket_servidor);

	return 0;

}

void inicializar_colas(){
	inicializar_cola(&QUEUE_NEW_POKEMON, NEW_POKEMON);
	inicializar_cola(&QUEUE_APPEARED_POKEMON, APPEARED_POKEMON);
	inicializar_cola(&QUEUE_CATCH_POKEMON, CATCH_POKEMON);
	inicializar_cola(&QUEUE_CAUGHT_POKEMON, CAUGHT_POKEMON);
	inicializar_cola(&QUEUE_GET_POKEMON, GET_POKEMON);
	inicializar_cola(&QUEUE_LOCALIZED_POKEMON, LOCALIZED_POKEMON);
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


void inicializar_cola(t_cola_de_mensajes** nombre_cola, queue_name cola){

	*nombre_cola = malloc(sizeof(t_cola_de_mensajes));
	(*nombre_cola)->cola = queue_create();
	(*nombre_cola)->lista_suscriptores = list_create();
	(*nombre_cola)->tipo_cola = cola;

}

t_cola_de_mensajes* int_a_nombre_cola(queue_name id){

	t_cola_de_mensajes* cola = NULL;

	switch (id){
		case NEW_POKEMON:
			cola = QUEUE_NEW_POKEMON;
			break;

		case APPEARED_POKEMON:
			cola = QUEUE_APPEARED_POKEMON;
			break;

		case CATCH_POKEMON:
			cola = QUEUE_CATCH_POKEMON;
			break;

		case CAUGHT_POKEMON:
			cola = QUEUE_CAUGHT_POKEMON;
			break;

		case GET_POKEMON:
			cola = QUEUE_GET_POKEMON;
			break;

		case LOCALIZED_POKEMON:
			cola = QUEUE_LOCALIZED_POKEMON;
			break;

		default:
			break;
}
	return cola;
}

void inicializar(){


	config = leer_config();
	logger = iniciar_logger();

	inicializar_colas();

	contador_id = 0;
	static const char* valores_colas[7] = {"NEW_POKEMON","APPEARED_POKEMON","CATCH_POKEMON","CAUGHT_POKEMON","GET_POKEMON","LOCALIZED_POKEMON", "PRODUCTOR"};
	memcpy(nombres_colas,valores_colas,sizeof(valores_colas));

	sockets_productores = list_create();
	pthread_mutex_init(&mutex_productores, NULL);
	pthread_mutex_init(&semaforo_suscriber,NULL);
	pthread_mutex_init(&semaforo_id,NULL);

	for(int i = 0; i <= 5; i++){
		pthread_mutex_init(&(sem_cola[i]),NULL);
	}
}

void estado_de_queues(){

	while(1){
		mostrar_estado_de_una_queue(QUEUE_NEW_POKEMON);
		mostrar_estado_de_una_queue(QUEUE_APPEARED_POKEMON);
		mostrar_estado_de_una_queue(QUEUE_CATCH_POKEMON);
		mostrar_estado_de_una_queue(QUEUE_CAUGHT_POKEMON);
		mostrar_estado_de_una_queue(QUEUE_GET_POKEMON);
		mostrar_estado_de_una_queue(QUEUE_LOCALIZED_POKEMON);
		sleep(6);
	}
}

void mostrar_estado_de_una_queue(t_cola_de_mensajes* cola){
	printf("%s\n", enum_to_string(cola->tipo_cola));
	printf("MENSAJES\n");
	recorrer_cola_de_mensajes_para_mostrar(cola);
	printf("\n");
	list_iterate(cola->lista_suscriptores,print_list_sockets);
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
	printf("MENSAJE RECIBIDO POR: %d\n", *(int*) numero);
}

void print_mensaje_de_cola(t_info_mensaje* mensaje){

	uint32_t id_mensaje = mensaje->id;
	printf("ID: %d\n",id_mensaje);

	queue_name id_cola = mensaje->paquete->cola_msg;
	void* msg = deserializar_buffer(id_cola,mensaje->paquete->buffer);

	print_msg(id_cola, msg);

	list_iterate(mensaje->a_quienes_fue_enviado,print_list_sockets_de_un_mensaje);

	printf("CUANTOS LO RECIBIERON: %d\n",mensaje->cuantos_lo_recibieron);
}

void free_mensaje(t_info_mensaje* mensaje){
	free_paquete(mensaje->paquete);
	list_destroy_and_destroy_elements(mensaje->a_quienes_fue_enviado,free);
	free(mensaje);
}
