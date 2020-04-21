#include <stdio.h>
#include "broker.h"

int main(){

	t_cola_de_mensajes NEW_POKEMON = inicializar_cola(NEW_POKEMON);

	t_cola_de_mensajes APPEARED_POKEMON = inicializar_cola(APPEARED_POKEMON);


	t_cola_de_mensajes CATCH_POKEMON = inicializar_cola(CATCH_POKEMON);


	t_cola_de_mensajes CAUGHT_POKEMON =	inicializar_cola(CAUGHT_POKEMON);


	t_cola_de_mensajes GET_POKEMON = inicializar_cola(GET_POKEMON);


	t_cola_de_mensajes LOCALIZED_POKEMON = inicializar_cola(LOCALIZED_POKEMON);


	//recibir_mensajes();

	return 0;

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
