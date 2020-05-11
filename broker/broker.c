#include "utils/broker.h"


int main(){

	inicializar();


	int socket_servidor = iniciar_servidor();

	pthread_t hilo_recibir;
	pthread_create(&hilo_recibir, NULL, (void*) esperar_cliente, &socket_servidor);
	pthread_join(hilo_recibir,NULL);

	for(int i = 0; i<6 ;i++){
		pthread_mutex_init(&(sem_cola[i]),NULL);
		cont_cola[i] = 0;
	}

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
		 printf("Error inicilizando logger");
		 exit(-1);
	 }

	 return mi_logger;
}


t_config* leer_config(){
	 t_config* mi_config = config_create("broker.config");
	 if(mi_config == NULL){
		 printf("Error inicilizando config");
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
}
	return cola;
}

void inicializar(){

	//config = leer_config();
	//logger = iniciar_logger();

	contador_id = 0;
	static const char* valores_colas[6] = {"NEW_POKEMON","APPEARED_POKEMON","CATCH_POKEMON","CAUGHT_POKEMON","GET_POKEMON","LOCALIZED_POKEMON"};
	memcpy(nombres_colas,valores_colas,sizeof(valores_colas));

	inicializar_colas();

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
