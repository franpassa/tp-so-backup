#include "broker.h"
#include "recibir.h"
#include "mandar.h"
#include "server.h"

void inicializar_colas(){
	QUEUE_NEW_POKEMON = inicializar_cola(QUEUE_NEW_POKEMON);
	QUEUE_APPEARED_POKEMON = inicializar_cola(QUEUE_APPEARED_POKEMON);
	QUEUE_CATCH_POKEMON = inicializar_cola(QUEUE_CATCH_POKEMON);
	QUEUE_CAUGHT_POKEMON =inicializar_cola(QUEUE_CAUGHT_POKEMON);
	QUEUE_GET_POKEMON = inicializar_cola(QUEUE_GET_POKEMON);
	QUEUE_LOCALIZED_POKEMON = inicializar_cola(QUEUE_LOCALIZED_POKEMON);
}

int main(){
	// ver archivos

	config = leer_config();
	logger = iniciar_logger();

	inicializar_colas();


	pthread_t hilo_servidor;

	pthread_create(&hilo_servidor,NULL,(void*) iniciar_servidor,NULL);


	pthread_join(hilo_servidor,NULL);

	//pthread_create(&hilo_recibir,NULL,(void*) recibir_mensajes,NULL);
	//pthread_create(&hilo_enviar,NULL,(void*) enviar_mensajes,NULL);
	//pthread_join(hilo_recibir, NULL);
	//pthread_join(hilo_enviar, NULL);

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
