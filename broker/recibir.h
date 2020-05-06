#ifndef RECIBIR_H_
#define RECIBIR_H_

#include "broker.h"


void recibir_mensajes(){ // ver socket por parametro

	int id_cola;
	int socket_cliente;
	t_paquete* paq = malloc(sizeof(t_paquete));

	while(1){

	// sino agregar void recibir_socket

	recv(socket_cliente, &(paq->cola_msg), sizeof(queue_name), MSG_WAITALL);


	id_cola = paq->cola_msg;

	if (paq->buffer->size != 0){

		paq->buffer = malloc(sizeof(t_buffer));
		recv(socket_cliente, &(paq->buffer->size), sizeof(queue_name), MSG_WAITALL);

		paq->buffer->stream = malloc(paq->buffer->size);
		recv(socket_cliente, paq->buffer->stream, paq->buffer->size, MSG_WAITALL);

		if (revisar_mensaje(id_cola,paq->buffer)){

			int id_mensaje = crear_nuevo_id();

			enviar_a_publisher_id(id_mensaje); // hacer

			agregar_a_cola(id_cola,paq);
		}
	}
	else {

		int id_correlativo = (int) paq->buffer->stream;

		confirmar_mensaje(id_cola , id_correlativo);
	}
	}
}

void confirmar_mensaje(int id_cola ,int id_mensaje){

	t_cola_de_mensajes queue = int_a_nombre_cola(id_cola);
	t_info_mensaje* mensaje = queue_peek(queue.cola);

	int control = 0;
	int id_primero = mensaje->id,id_siguiente;

	do{
		mensaje = queue_pop(queue.cola);

		if (mensaje->id == id_mensaje){
			control = 1;
			mensaje->cuantos_lo_recibieron++;
		}

		queue_push(queue.cola ,mensaje);

		mensaje = queue_peek(queue.cola);

		id_siguiente = mensaje->id;

	}while( control == 0 && id_primero != id_siguiente);
}

int crear_nuevo_id(){

	pthread_mutex_lock(&semaforo_id);
	contador_id ++;
	return contador_id;
	pthread_mutex_unlock(&semaforo_id);
}

void agregar_a_cola(int id_cola,t_paquete* paquete){

	void* msg = &paquete;

	queue_push(int_a_nombre_cola(id_cola).cola, msg);
}

bool es_el_mismo_mensaje(int id, void* mensaje,void* otro_mensaje) {

	switch(id){

	case 1: ;

		new_pokemon_msg* msg_new = (new_pokemon_msg*) mensaje;
		new_pokemon_msg* otro_msg_new = (new_pokemon_msg*) otro_mensaje;
		return (msg_new->cantidad_pokemon == otro_msg_new->cantidad_pokemon && msg_new->coordenada_X == otro_msg_new->coordenada_X
				&& msg_new->coordenada_Y == otro_msg_new->coordenada_Y && msg_new->nombre_pokemon == otro_msg_new->nombre_pokemon
				&& msg_new->tamanio_nombre == otro_msg_new->tamanio_nombre);

		break;

	case 2: ;

		appeared_pokemon_msg* msg_appeared = (appeared_pokemon_msg*) mensaje;
		appeared_pokemon_msg* otro_msg_appeared = (appeared_pokemon_msg*) otro_mensaje;
		return (msg_appeared->coordenada_X == otro_msg_appeared->coordenada_X && msg_appeared->coordenada_Y == otro_msg_appeared->coordenada_Y
				&& msg_appeared->nombre_pokemon == otro_msg_appeared->nombre_pokemon && msg_appeared->tamanio_nombre == otro_msg_appeared->tamanio_nombre);

		break;

	case 3: ;

		get_pokemon_msg* msg_get = (get_pokemon_msg*) mensaje;
		get_pokemon_msg* otro_msg_get = (get_pokemon_msg*) otro_mensaje;
		return (msg_get->nombre_pokemon == otro_msg_get->nombre_pokemon && msg_get->tamanio_nombre == otro_msg_get->tamanio_nombre);

		break;

	case 4: ;

		localized_pokemon_msg* msg_localized = (localized_pokemon_msg*) mensaje;
		localized_pokemon_msg* otro_msg_localized = (localized_pokemon_msg*) otro_mensaje;
		return (msg_localized->cantidad_posiciones == otro_msg_localized->cantidad_posiciones && msg_localized->id_correlativo == otro_msg_localized->id_correlativo
				&& msg_localized->nombre_pokemon == otro_msg_localized->nombre_pokemon && msg_localized->pares_coordenadas == otro_msg_localized->pares_coordenadas
				&& msg_localized->tamanio_nombre == otro_msg_localized->tamanio_nombre);

		break;

	case 5: ;

		catch_pokemon_msg* msg_catch = (catch_pokemon_msg*) mensaje;
		catch_pokemon_msg* otro_msg_catch = (catch_pokemon_msg*) otro_mensaje;
		return (msg_catch->coordenada_X == otro_msg_catch->coordenada_X && msg_catch->coordenada_Y == otro_msg_catch->coordenada_Y
				&& msg_catch->nombre_pokemon == otro_msg_catch->nombre_pokemon
				&& msg_catch->tamanio_nombre == otro_msg_catch->tamanio_nombre);

		break;

	case 6: ;

	    caught_pokemon_msg* msg_caught = (caught_pokemon_msg*) mensaje;
		caught_pokemon_msg* otro_msg_caught = (caught_pokemon_msg*) otro_mensaje;
		return (msg_caught->id_correlativo == otro_msg_caught->id_correlativo && msg_caught->resultado == otro_msg_caught->resultado);

		break;

	default:

		return false;
	}

}

bool revisar_mensaje(int id, t_buffer* buffer) {

	t_cola_de_mensajes queue_a_revisar = int_a_nombre_cola(id);

	bool no_es_el_mismo_mensaje(void* elemento){
		return !es_el_mismo_mensaje(id, buffer->stream, elemento);
	}

	bool resultado = list_any_satisfy(queue_a_revisar.cola->elements, no_es_el_mismo_mensaje);

	return resultado;
}


#endif /* RECIBIR_H_ */
