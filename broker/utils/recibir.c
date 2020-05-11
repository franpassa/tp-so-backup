#include "broker.h"


void loop_productores(){

	while(1){
		if(list_size(sockets_productores) > 0){
			list_iterate(sockets_productores, (void*) chequear_mensajes);
			sleep(8);
		}
	}

}

void chequear_mensajes(int* socket_escucha){

	queue_name id_cola;
	t_paquete* paq = malloc(sizeof(t_paquete));
	int bytes_recibidos = recv(*socket_escucha, &id_cola, sizeof(queue_name), MSG_DONTWAIT);
	if(bytes_recibidos < 0){
		perror("hola");
		printf("EL SOCKET %d NO ME MANDO NADA, RE GIL\n", *socket_escucha);
		return;
	} else {
		printf("bytes recibidos %d\n", bytes_recibidos);
	}

	new_pokemon_msg* msg = recibir_mensaje(id_cola, *socket_escucha);
	printf("pokemon recibido: nombre %s X: %d Y: %d Cantidad: %d\n", msg->nombre_pokemon, msg->coordenada_X, msg->coordenada_Y, msg->cantidad_pokemon);
	int id_ponele = 3423;
	send(*socket_escucha, &id_ponele, sizeof(int), 0);

	/*if (paq->buffer->size != 0){

		paq->buffer = malloc(sizeof(t_buffer));
		recv(*socket_escucha, &(paq->buffer->size), sizeof(queue_name), MSG_WAITALL);

		paq->buffer->stream = malloc(paq->buffer->size);
		recv(*socket_escucha, paq->buffer->stream, paq->buffer->size, MSG_WAITALL);

		if (revisar_mensaje(id_cola,paq->buffer)){

			uint32_t id_mensaje = crear_nuevo_id();

			enviar_a_publisher_id(id_mensaje); // hacer

			pthread_mutex_lock(&(sem_cola[id_cola]));
			agregar_a_cola(id_cola,paq);
			pthread_mutex_unlock(&(sem_cola[id_cola]));

			cont_cola[id_cola] = 1;
		}
	} else {

		uint32_t id_correlativo = (uint32_t) paq->buffer->stream;

		pthread_mutex_lock(&(sem_cola[id_cola]));
		confirmar_mensaje(id_cola , id_correlativo);
		pthread_mutex_unlock(&(sem_cola[id_cola]));
	}*/
}

void confirmar_mensaje(queue_name id_cola ,uint32_t id_mensaje){

	t_cola_de_mensajes* queue = int_a_nombre_cola(id_cola);
	t_info_mensaje* mensaje = queue_peek(queue->cola);

	uint32_t control = 0;
	uint32_t id_primero = mensaje->id,id_siguiente;

	do{
		mensaje = queue_pop(queue->cola);

		if (mensaje->id == id_mensaje){
			control = 1;
			mensaje->cuantos_lo_recibieron++;
		}
		// eliminar mensajes si ya recibieron todos
		queue_push(queue->cola ,mensaje);

		mensaje = queue_peek(queue->cola);

		id_siguiente = mensaje->id;

	}while( control == 0 && id_primero != id_siguiente);
}

int crear_nuevo_id(){

	pthread_mutex_lock(&semaforo_id);
	contador_id ++;
	return contador_id;
	pthread_mutex_unlock(&semaforo_id);
}

void agregar_a_cola(uint32_t id_cola,t_paquete* paquete){

	void* msg = &paquete;

	queue_push(int_a_nombre_cola(id_cola)->cola, msg);
}

bool es_el_mismo_mensaje(queue_name id, void* mensaje,void* otro_mensaje) {

	switch(id){

	case NEW_POKEMON: ;

		new_pokemon_msg* msg_new = (new_pokemon_msg*) mensaje;
		new_pokemon_msg* otro_msg_new = (new_pokemon_msg*) otro_mensaje;
		return (msg_new->cantidad_pokemon == otro_msg_new->cantidad_pokemon && msg_new->coordenada_X == otro_msg_new->coordenada_X
				&& msg_new->coordenada_Y == otro_msg_new->coordenada_Y && msg_new->nombre_pokemon == otro_msg_new->nombre_pokemon
				&& msg_new->tamanio_nombre == otro_msg_new->tamanio_nombre);

		break;

	case APPEARED_POKEMON: ;

		appeared_pokemon_msg* msg_appeared = (appeared_pokemon_msg*) mensaje;
		appeared_pokemon_msg* otro_msg_appeared = (appeared_pokemon_msg*) otro_mensaje;
		return (msg_appeared->coordenada_X == otro_msg_appeared->coordenada_X && msg_appeared->coordenada_Y == otro_msg_appeared->coordenada_Y
				&& msg_appeared->nombre_pokemon == otro_msg_appeared->nombre_pokemon && msg_appeared->tamanio_nombre == otro_msg_appeared->tamanio_nombre);

		break;

	case GET_POKEMON: ;

		get_pokemon_msg* msg_get = (get_pokemon_msg*) mensaje;
		get_pokemon_msg* otro_msg_get = (get_pokemon_msg*) otro_mensaje;
		return (msg_get->nombre_pokemon == otro_msg_get->nombre_pokemon && msg_get->tamanio_nombre == otro_msg_get->tamanio_nombre);

		break;

	case LOCALIZED_POKEMON: ;

		localized_pokemon_msg* msg_localized = (localized_pokemon_msg*) mensaje;
		localized_pokemon_msg* otro_msg_localized = (localized_pokemon_msg*) otro_mensaje;
		return (msg_localized->cantidad_posiciones == otro_msg_localized->cantidad_posiciones && msg_localized->id_correlativo == otro_msg_localized->id_correlativo
				&& msg_localized->nombre_pokemon == otro_msg_localized->nombre_pokemon && msg_localized->pares_coordenadas == otro_msg_localized->pares_coordenadas
				&& msg_localized->tamanio_nombre == otro_msg_localized->tamanio_nombre);

		break;

	case CATCH_POKEMON: ;

		catch_pokemon_msg* msg_catch = (catch_pokemon_msg*) mensaje;
		catch_pokemon_msg* otro_msg_catch = (catch_pokemon_msg*) otro_mensaje;
		return (msg_catch->coordenada_X == otro_msg_catch->coordenada_X && msg_catch->coordenada_Y == otro_msg_catch->coordenada_Y
				&& msg_catch->nombre_pokemon == otro_msg_catch->nombre_pokemon
				&& msg_catch->tamanio_nombre == otro_msg_catch->tamanio_nombre);

		break;

	case CAUGHT_POKEMON: ;

	    caught_pokemon_msg* msg_caught = (caught_pokemon_msg*) mensaje;
		caught_pokemon_msg* otro_msg_caught = (caught_pokemon_msg*) otro_mensaje;
		return (msg_caught->id_correlativo == otro_msg_caught->id_correlativo && msg_caught->resultado == otro_msg_caught->resultado);

		break;

	default:

		return false;
	}

}

bool revisar_mensaje(uint32_t id, t_buffer* buffer) {

	t_cola_de_mensajes* queue_a_revisar = int_a_nombre_cola(id);

	bool no_es_el_mismo_mensaje(void* elemento){
		return !es_el_mismo_mensaje(id, buffer->stream, elemento);
	}

	bool resultado = list_any_satisfy(queue_a_revisar->cola->elements, no_es_el_mismo_mensaje);

	return resultado;
}

void enviar_a_publisher_id(uint32_t id){
	// hacer
};



