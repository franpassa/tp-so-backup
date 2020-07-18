#include "broker.h"


void loop_productores(){

	while(1){
		if(!list_is_empty(sockets_productores)){
			list_iterate(sockets_productores, (void*) recibir_mensajes_para_broker);
			sleep(2);
		}
	}

}

void recibir_mensajes_para_broker(int* socket_escucha){

	queue_name id_cola;
	int bytes_recibidos = recv(*socket_escucha, &id_cola, sizeof(queue_name), MSG_DONTWAIT);

	if(bytes_recibidos < 0){
		printf("EL SOCKET %d NO ME MANDO NADA, RE ORTIBA\n", *socket_escucha);
		return;
	} else if(bytes_recibidos == 0){
		printf("EL SOCKET %d SE DESCONECTO\n", *socket_escucha);
		return;
	}

	t_paquete* paquete= malloc(sizeof(t_paquete));
	paquete->cola_msg = id_cola;

	paquete->buffer = malloc(sizeof(t_buffer));
	recv(*socket_escucha, &(paquete->buffer->size), sizeof(uint32_t), MSG_WAITALL);
	//printf("tamanio_buffer:%d ",paquete->buffer->size);

	if (paquete->buffer->size != 0){

		paquete->buffer->stream = malloc(paquete->buffer->size);
		recv(*socket_escucha, paquete->buffer->stream, paquete->buffer->size, MSG_WAITALL);
		void* stream_a_comparar = malloc(paquete->buffer->size);
		memcpy(stream_a_comparar, paquete->buffer->stream, paquete->buffer->size);

		int id_mens_en_cola = revisar_si_mensaje_no_estaba_en_cola(id_cola, stream_a_comparar, paquete->buffer->size);

		if (id_mens_en_cola == 0){ // Si es 0 no esta en la cola el msg

			pthread_mutex_lock(&semaforo_id);
 			uint32_t id_mensaje = crear_nuevo_id();
			pthread_mutex_unlock(&semaforo_id);

			send(*socket_escucha, &id_mensaje, sizeof(uint32_t), 0);

			printf("MENSAJE NUEVO -- ID: %d -- COLA: %s\n",id_mensaje, nombres_colas[id_cola]);
			//log_info(logger, " MENSAJE NUEVO: %s -- ID: %d -- COLA: %s ", msg_as_string(id_cola, msg), id_mensaje, nombres_colas[id_cola]); // LOG 3 hay que deserializarlo para mostrarlo

			pthread_mutex_lock(&(sem_cola[id_cola]));
			agregar_a_cola(id_cola, id_mensaje);
			pthread_mutex_unlock(&(sem_cola[id_cola]));

			almacenar(paquete->buffer->stream, id_cola, id_mensaje, paquete->buffer->size);

		} else {
			send(*socket_escucha, &id_mens_en_cola, sizeof(uint32_t), 0);
		}

	} else {

		uint32_t id_correlativo;
		recv(*socket_escucha, &id_correlativo, sizeof(uint32_t), MSG_WAITALL);

		uint32_t socket_sub; // Falta que me manden el socket_sub
		recv(*socket_escucha, &socket_sub, sizeof(uint32_t), MSG_WAITALL);

		pthread_mutex_lock(&(sem_cola[id_cola]));
		confirmar_mensaje(id_cola, id_correlativo, socket_sub);
		pthread_mutex_unlock(&(sem_cola[id_cola]));

	}
	list_remove_and_destroy_element(sockets_productores,0,free);

}


void confirmar_mensaje(queue_name id_cola, uint32_t id_mensaje, int socket_sub) { // Revisarlo

	t_cola_de_mensajes* queue = int_a_nombre_cola(id_cola);
	t_info_mensaje* mensaje = queue_peek(queue->cola);

	uint32_t control = 0;
	uint32_t id_primero = mensaje->id;
	uint32_t id_siguiente;

	do {
		mensaje = queue_pop(queue->cola);

		if (mensaje->id == id_mensaje) {
			control = 1;

			uint32_t *sub = malloc(sizeof(uint32_t));
			*sub = socket_sub;

			if (!esta_en_lista(mensaje->quienes_lo_recibieron, sub)) {
				list_add(mensaje->quienes_lo_recibieron, sub);
			}
			log_info(logger,"CONFIRMACION DE LLEGADA DE MENSAJE DE SUSCRIPTOR:%d ",sub); // LOG 5
			if (list_size(mensaje->quienes_lo_recibieron) == list_size(queue->lista_suscriptores)) {
				free_msg_cola(mensaje);
				//eliminar_mensaje(mensaje->id);
			}

		} else {
			queue_push(queue->cola, mensaje);
			mensaje = queue_peek(queue->cola);
			id_siguiente = mensaje->id;
		}

	} while (control == 0 && id_primero != id_siguiente);
}

uint32_t crear_nuevo_id(){
	contador_id ++;
	return contador_id;
}

void agregar_a_cola(uint32_t id_cola, uint32_t id_mensaje){
	t_info_mensaje* info_msg = malloc(sizeof(t_info_mensaje));
	info_msg->id = id_mensaje;
	info_msg->quienes_lo_recibieron = list_create();
	info_msg->a_quienes_fue_enviado = list_create();
	t_cola_de_mensajes* queue_del_mensaje_a_pushear = int_a_nombre_cola(id_cola);
	queue_push(queue_del_mensaje_a_pushear->cola, info_msg);
}

bool es_el_mismo_mensaje(queue_name id, void* mensaje, void* otro_mensaje) {

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
		return (string_equals_ignore_case(msg_get->nombre_pokemon, otro_msg_get->nombre_pokemon) && msg_get->tamanio_nombre == otro_msg_get->tamanio_nombre);

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

int revisar_si_mensaje_no_estaba_en_cola(queue_name id, void* msg_recibido, uint32_t tamanio_mensaje) {

	t_cola_de_mensajes* queue_a_revisar = int_a_nombre_cola(id);

	int mensaje_nuevo = 0;

	t_buffer* mensaje_en_buffer_recibido = malloc(sizeof(t_buffer));
	mensaje_en_buffer_recibido->stream = msg_recibido;
	mensaje_en_buffer_recibido->size = tamanio_mensaje;
	void* msg_a_comparar = deserializar_buffer(id, mensaje_en_buffer_recibido);
	void* msg;
	void* msg2;
	t_info_mensaje* elemento_a_testear;

	pthread_mutex_lock(&(sem_cola[id]));
	if (!queue_is_empty(queue_a_revisar->cola)) {

		for (int i = 0; i < queue_size(queue_a_revisar->cola); i++) {
			elemento_a_testear = queue_pop(queue_a_revisar->cola);
			t_buffer* mensaje_en_cola_buffer = malloc(sizeof(t_buffer));
			msg = de_id_mensaje_a_mensaje(elemento_a_testear->id);
			mensaje_en_cola_buffer->stream = msg;
			mensaje_en_cola_buffer->size = de_id_mensaje_a_size(elemento_a_testear->id);
			msg2 = deserializar_buffer(id, mensaje_en_cola_buffer);

			if (es_el_mismo_mensaje(id, msg2, msg_a_comparar)) {
				mensaje_nuevo = elemento_a_testear->id; // asignas el id del que ya esta en la cola y se lo das al sub
			}
			queue_push(queue_a_revisar->cola, elemento_a_testear);
			free(mensaje_en_cola_buffer->stream);
			free(mensaje_en_cola_buffer);
		}

	}
	pthread_mutex_unlock(&(sem_cola[id]));

	free(mensaje_en_buffer_recibido->stream);
	free(mensaje_en_buffer_recibido);


	return mensaje_nuevo;
}
