#include <stdio.h>
#include "broker.h"

int main(){

	NEW_POKEMON = inicializar_cola(NEW_POKEMON);
	APPEARED_POKEMON = inicializar_cola(APPEARED_POKEMON);
	CATCH_POKEMON = inicializar_cola(CATCH_POKEMON);
	CAUGHT_POKEMON =inicializar_cola(CAUGHT_POKEMON);
	GET_POKEMON = inicializar_cola(GET_POKEMON);
	LOCALIZED_POKEMON = inicializar_cola(LOCALIZED_POKEMON);

	pthread_t hilo_recibir, hilo_enviar;

	pthread_create(&hilo_recibir,NULL,(void*) recibir_mensajes,NULL);
	pthread_create(&hilo_enviar,NULL,(void*) enviar_mensajes,NULL);

	pthread_join(hilo_recibir, NULL);
	pthread_join(hilo_enviar, NULL);
	return 0;

}

void agregar_a_cola (int id_cola, t_info_mensaje mensaje){

	void* msg = &mensaje;

	switch (id_cola)
	{
	case 1:
			queue_push(NEW_POKEMON.cola, msg);
			break;
	case 2:
			queue_push(APPEARED_POKEMON.cola, msg);
			break;
	case 3:
			queue_push(CATCH_POKEMON.cola, msg);
			break;
	case 4:
			queue_push(CAUGHT_POKEMON.cola, msg);
			break;
	case 5:
			queue_push(GET_POKEMON.cola, msg);
			break;
	case 6:
			queue_push(LOCALIZED_POKEMON.cola, msg);
			break;
	}
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
