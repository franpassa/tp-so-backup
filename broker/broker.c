#include "broker.h"
#include "recibir.h"
#include "mandar.h"



int main(void){

	QUEUE_NEW_POKEMON = inicializar_cola(QUEUE_NEW_POKEMON);
	QUEUE_APPEARED_POKEMON = inicializar_cola(QUEUE_APPEARED_POKEMON);
	QUEUE_CATCH_POKEMON = inicializar_cola(QUEUE_CATCH_POKEMON);
	QUEUE_CAUGHT_POKEMON =inicializar_cola(QUEUE_CAUGHT_POKEMON);
	QUEUE_GET_POKEMON = inicializar_cola(QUEUE_GET_POKEMON);
	QUEUE_LOCALIZED_POKEMON = inicializar_cola(QUEUE_LOCALIZED_POKEMON);

	/*char* ip;
	char* puerto;
	logger = iniciar_logger();

	log_info(logger,"Broker");

	config = leer_config();

	ip = config_get_string_value(config,"IP_BROKER");
	puerto = config_get_string_value(config,"PUERTO_BROKER");

	log_info(logger,ip);
	log_info(logger,puerto);

	terminar_programa(logger,config);
	free(ip);
	free(puerto);

	iniciar_servidor();

	//pthread_t hilo_recibir, hilo_enviar;

	//pthread_create(&hilo_recibir,NULL,(void*) recibir_mensajes,NULL);
	//pthread_create(&hilo_enviar,NULL,(void*) enviar_mensajes,NULL);

	//pthread_join(hilo_recibir, NULL);
	//pthread_join(hilo_enviar, NULL);
	*/
	return 0;

}


t_log* iniciar_logger(void){
	return log_create("log", "broker", 1, LOG_LEVEL_INFO);
}


t_config* leer_config(void){
	 return config_create("broker.config");
}

void terminar_programa(t_log* logger, t_config* config){
		log_destroy(logger);
		config_destroy(config);
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
