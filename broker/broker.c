#include "utils/broker.h"


int main(){

	inicializar();

	int socket_servidor = iniciar_servidor_broker();

	pthread_t hilo_estado_queues;
	pthread_create(&hilo_estado_queues,NULL,(void*) estado_de_queues,&socket_servidor);

	pthread_t hilo_suscripciones;
	pthread_create(&hilo_suscripciones, NULL, (void*) esperar_cliente, &socket_servidor);

	pthread_t hilo_mensajes;
	pthread_create(&hilo_mensajes, NULL, (void*) loop_productores, NULL);

	pthread_join(hilo_estado_queues,NULL);
	pthread_join(hilo_suscripciones,NULL);
	pthread_join(hilo_mensajes,NULL);

	return 0;

}

void inicializar_colas(){
	inicializar_cola(&QUEUE_NEW_POKEMON);
	inicializar_cola(&QUEUE_APPEARED_POKEMON);
	inicializar_cola(&QUEUE_CATCH_POKEMON);
	inicializar_cola(&QUEUE_CAUGHT_POKEMON);
	inicializar_cola(&QUEUE_GET_POKEMON);
	inicializar_cola(&QUEUE_LOCALIZED_POKEMON);
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


void inicializar_cola(t_cola_de_mensajes** nombre_cola){

	*nombre_cola = malloc(sizeof(t_cola_de_mensajes));
	(*nombre_cola)->cola = queue_create();
	(*nombre_cola)->lista_suscriptores = list_create();

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

		case PRODUCTOR:
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


	for(int i = 0; i<6 ;i++){
		pthread_mutex_init(&(sem_cola[i]),NULL);
		cont_cola[i] = 0;
	}
}

void estado_de_queues(){

	while(1){
	mostrar_subs(QUEUE_NEW_POKEMON);
	mostrar_subs(QUEUE_APPEARED_POKEMON);
	mostrar_subs(QUEUE_CATCH_POKEMON);
	mostrar_subs(QUEUE_CAUGHT_POKEMON);
	mostrar_subs(QUEUE_GET_POKEMON);
	mostrar_subs(QUEUE_LOCALIZED_POKEMON);
	sleep(4);
	}
}

void mostrar_subs(t_cola_de_mensajes* cola){
	list_iterate(cola->lista_suscriptores,print_list_sockets);
}

void print_list_sockets(void* numero){
	printf("socket sub: %d\n", (*(int*) numero));
}

/*t_cola_de_mensajes nuevo;
	nuevo = inicializar_cola();
	t_suscriptor uno;
	uno.id=4;
	uno.id_correlacional=2;
	void* p =&uno;
	int posicion = list_add(nuevo.lista_suscriptores, p);
	void* p2 = list_get(nuevo.lista_suscriptores,posicion);
	t_suscriptor sub = *(t_suscriptor*)p2;
	printf("%d\n",sub.id);
	printf("%d\n",sub.id_correlacional);
*/
