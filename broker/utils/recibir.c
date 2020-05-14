#include "broker.h"


void loop_productores(){

	while(1){
		if(list_size(sockets_productores) > 0){
			list_iterate(sockets_productores, (void*) chequear_mensajes);
			sleep(2);
		}
	}

}

void chequear_mensajes(int* socket_escucha){

	queue_name id_cola;
	int bytes_recibidos = recv(*socket_escucha, &id_cola, sizeof(queue_name), MSG_DONTWAIT);

	if(bytes_recibidos < 0){
		printf("EL SOCKET %d NO ME MANDO NADA, RE ORTIBA\n", *socket_escucha);
		return;
	} else if(bytes_recibidos == 0){
		printf("EL SOCKET %d SE DESCONECTO\n", *socket_escucha);
		return;
	}

	printf("MENSAJE DE SOCKET %d -> ", *socket_escucha);

	switch(id_cola){

		case NEW_POKEMON: ;
			new_pokemon_msg* msg_new = recibir_mensaje(id_cola, *socket_escucha);
			printf("NEW POKEMON {nombre: %s, X: %d, Y: %d, cantidad: %d}\n", msg_new->nombre_pokemon, msg_new->coordenada_X, msg_new->coordenada_Y, msg_new->cantidad_pokemon);
			break;

		case APPEARED_POKEMON: ;
			appeared_pokemon_msg* msg_appeared = recibir_mensaje(id_cola, *socket_escucha);
			printf("APPEARED POKEMON {nombre: %s, X: %d, Y: %d}\n", msg_appeared->nombre_pokemon, msg_appeared->coordenada_X, msg_appeared->coordenada_Y);
			break;

		case GET_POKEMON: ;
			get_pokemon_msg* msg_get = recibir_mensaje(id_cola, *socket_escucha);
			printf("GET POKEMON {nombre: %s}\n", msg_get->nombre_pokemon);
			break;

		case LOCALIZED_POKEMON: ;
			localized_pokemon_msg* msg_localized = recibir_mensaje(id_cola, *socket_escucha);
			printf("LOCALIZED POKEMON {id correlativo: %d, nombre: %s, cantidad posiciones: %d, paja printear los pares de coordenadas}\n", msg_localized->id_correlativo, msg_localized->nombre_pokemon, msg_localized->cantidad_posiciones);
			break;

		case CATCH_POKEMON: ;
			catch_pokemon_msg* msg_catch = recibir_mensaje(id_cola, *socket_escucha);
			printf("CATCH POKEMON {nombre: %s, X: %d, Y: %d}\n", msg_catch->nombre_pokemon, msg_catch->coordenada_X, msg_catch->coordenada_Y);
			break;

		case CAUGHT_POKEMON: ;
			caught_pokemon_msg* msg_caught = recibir_mensaje(id_cola, *socket_escucha);
			printf("CAUGHT POKEMON {id correlativo: %d, resultado: %d}\n", msg_caught->id_correlativo, msg_caught->resultado);
			break;

		default: ;
			return;

	}

	uint32_t id_msg = crear_nuevo_id();

	send(*socket_escucha,&id_msg,sizeof(uint32_t),0);

//	t_paquete* paquete= malloc(sizeof(t_paquete));
//	paquete->cola_msg = id_cola;
//
//	paquete->buffer = malloc(sizeof(t_buffer));
//	recv(*socket_escucha, paquete->buffer, sizeof(t_buffer),MSG_WAITALL);
//
//	if (paquete->buffer->size != 0){
//
//		paquete->buffer->stream = malloc(paquete->buffer->size);
//		recv(*socket_escucha, paquete->buffer->stream, paquete->buffer->size, MSG_WAITALL);
//
//		if (revisar_si_mensaje_no_estaba_en_cola(id_cola,paquete->buffer->stream)){ // No entra en este if
//
//			uint32_t id_mensaje = crear_nuevo_id();
//
//			send(*socket_escucha,&id_mensaje,sizeof(uint32_t),0);
//
//			pthread_mutex_lock(&(sem_cola[id_cola]));
//			agregar_a_cola(id_cola,paquete);
//			pthread_mutex_unlock(&(sem_cola[id_cola]));
//
//			cont_cola[id_cola] = 1;
//
//		}
//	} else {
//
//		int id_correlativo = (int) paquete->buffer->stream ;
//
//		pthread_mutex_lock(&(sem_cola[id_cola]));
//		confirmar_mensaje(id_cola , id_correlativo);
//		pthread_mutex_unlock(&(sem_cola[id_cola]));
//	}


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

uint32_t crear_nuevo_id(){

	pthread_mutex_lock(&semaforo_id);
	contador_id ++;
	pthread_mutex_unlock(&semaforo_id);
	return contador_id;
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
				&& msg_new->coordenada_Y == otro_msg_new->coordenada_Y && string_equals_ignore_case(msg_new->nombre_pokemon,otro_msg_new->nombre_pokemon)
				&& msg_new->tamanio_nombre == otro_msg_new->tamanio_nombre);

		break;

	case APPEARED_POKEMON: ;

		appeared_pokemon_msg* msg_appeared = (appeared_pokemon_msg*) mensaje;
		appeared_pokemon_msg* otro_msg_appeared = (appeared_pokemon_msg*) otro_mensaje;
		return (msg_appeared->coordenada_X == otro_msg_appeared->coordenada_X && msg_appeared->coordenada_Y == otro_msg_appeared->coordenada_Y
				&& string_equals_ignore_case(msg_appeared->nombre_pokemon,otro_msg_appeared->nombre_pokemon) && msg_appeared->tamanio_nombre == otro_msg_appeared->tamanio_nombre);

		break;

	case GET_POKEMON: ;

		get_pokemon_msg* msg_get = (get_pokemon_msg*) mensaje;
		get_pokemon_msg* otro_msg_get = (get_pokemon_msg*) otro_mensaje;
		return (string_equals_ignore_case(msg_get->nombre_pokemon,otro_msg_get->nombre_pokemon) && msg_get->tamanio_nombre == otro_msg_get->tamanio_nombre);

		break;

	case LOCALIZED_POKEMON: ;

		localized_pokemon_msg* msg_localized = (localized_pokemon_msg*) mensaje;
		localized_pokemon_msg* otro_msg_localized = (localized_pokemon_msg*) otro_mensaje;
		return (msg_localized->cantidad_posiciones == otro_msg_localized->cantidad_posiciones && msg_localized->id_correlativo == otro_msg_localized->id_correlativo
				&& string_equals_ignore_case(msg_localized->nombre_pokemon,otro_msg_localized->nombre_pokemon) && msg_localized->pares_coordenadas == otro_msg_localized->pares_coordenadas
				&& msg_localized->tamanio_nombre == otro_msg_localized->tamanio_nombre);

		break;

	case CATCH_POKEMON: ;

		catch_pokemon_msg* msg_catch = (catch_pokemon_msg*) mensaje;
		catch_pokemon_msg* otro_msg_catch = (catch_pokemon_msg*) otro_mensaje;
		return (msg_catch->coordenada_X == otro_msg_catch->coordenada_X && msg_catch->coordenada_Y == otro_msg_catch->coordenada_Y
				&& string_equals_ignore_case(msg_catch->nombre_pokemon,otro_msg_catch->nombre_pokemon)
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

bool revisar_si_mensaje_no_estaba_en_cola(queue_name id, void* msg_en_buffer) {

	t_cola_de_mensajes* queue_a_revisar = int_a_nombre_cola(id);

	bool no_es_el_mismo_mensaje(void* elemento){
		return !es_el_mismo_mensaje(id, msg_en_buffer, elemento);
	}

		return list_any_satisfy(queue_a_revisar->cola->elements, no_es_el_mismo_mensaje);

}




