#include "broker.h"

void mandar_mensajes() {

	int cola_actual = 0;

	while (1) {
		if (cola_actual == 6) {
			cola_actual = 0;
		}
		if (queue_size(int_a_nombre_cola(cola_actual)->cola) != 0) {
			pthread_mutex_lock(&(sem_cola[cola_actual]));
			recorrer_cola(int_a_nombre_cola(cola_actual));
			pthread_mutex_unlock(&(sem_cola[cola_actual]));
		}

		cola_actual++;
	}
}

int mandar(queue_name cola, void* estructura_mensaje, int id ,int socket_receptor) {
	int control = 0;
	t_paquete* paquete = malloc(sizeof(t_paquete));

	paquete->cola_msg = cola;
	paquete->buffer = malloc(sizeof(t_buffer));
	void* stream;
	int offset = 0;

	switch (cola) {
	case NEW_POKEMON:;
		new_pokemon_msg* msg_new = (new_pokemon_msg*) estructura_mensaje;

		paquete->buffer->size = sizeof(uint32_t) * 4 + msg_new->tamanio_nombre;
		stream = malloc(paquete->buffer->size);

		memcpy(stream + offset, &(msg_new->tamanio_nombre), sizeof(uint32_t));
		offset += sizeof(uint32_t);
		memcpy(stream + offset, msg_new->nombre_pokemon,
				msg_new->tamanio_nombre);
		offset += msg_new->tamanio_nombre;
		memcpy(stream + offset, &(msg_new->coordenada_X), sizeof(uint32_t));
		offset += sizeof(uint32_t);
		memcpy(stream + offset, &(msg_new->coordenada_Y), sizeof(uint32_t));
		offset += sizeof(uint32_t);
		memcpy(stream + offset, &(msg_new->cantidad_pokemon), sizeof(uint32_t));

		break;

	case APPEARED_POKEMON:;
		appeared_pokemon_msg* msg_appeared = (appeared_pokemon_msg*) estructura_mensaje;

		paquete->buffer->size = sizeof(uint32_t) * 3 + msg_appeared->tamanio_nombre;
		stream = malloc(paquete->buffer->size);

		memcpy(stream + offset, &(msg_appeared->tamanio_nombre), sizeof(uint32_t));
		offset += sizeof(uint32_t);
		memcpy(stream + offset, msg_appeared->nombre_pokemon,
				msg_appeared->tamanio_nombre);
		offset += msg_appeared->tamanio_nombre;
		memcpy(stream + offset, &(msg_appeared->coordenada_X), sizeof(uint32_t));
		offset += sizeof(uint32_t);
		memcpy(stream + offset, &(msg_appeared->coordenada_Y),
				sizeof(uint32_t));

		break;

	case GET_POKEMON:;
		get_pokemon_msg* msg_get = (get_pokemon_msg*) estructura_mensaje;

		paquete->buffer->size = sizeof(uint32_t) * 2 + msg_get->tamanio_nombre;
		stream = malloc(paquete->buffer->size);

		memcpy(stream + offset, &(msg_get->tamanio_nombre), sizeof(uint32_t));
		offset += sizeof(uint32_t);
		memcpy(stream + offset, msg_get->nombre_pokemon, msg_get->tamanio_nombre);

		break;

	case LOCALIZED_POKEMON:;
		localized_pokemon_msg* msg_localized = (localized_pokemon_msg*) estructura_mensaje;

		uint32_t tamanio_pares_coordenadas = sizeof(uint32_t) * msg_localized->cantidad_posiciones * 2;

		paquete->buffer->size = sizeof(uint32_t) * 3 + tamanio_pares_coordenadas + msg_localized->tamanio_nombre;
		stream = malloc(paquete->buffer->size);

		memcpy(stream + offset, &(msg_localized->id_correlativo), sizeof(uint32_t));
		offset += sizeof(uint32_t);
		memcpy(stream + offset, &(msg_localized->tamanio_nombre), sizeof(uint32_t));
		offset += sizeof(uint32_t);
		memcpy(stream + offset, msg_localized->nombre_pokemon, msg_localized->tamanio_nombre);
		offset += msg_localized->tamanio_nombre;
		memcpy(stream + offset, &(msg_localized->cantidad_posiciones), sizeof(uint32_t));
		offset += sizeof(uint32_t);
		memcpy(stream + offset, msg_localized->pares_coordenadas, tamanio_pares_coordenadas);

		break;

	case CATCH_POKEMON:;
		catch_pokemon_msg* msg_catch = (catch_pokemon_msg*) estructura_mensaje;

		paquete->buffer->size = sizeof(uint32_t) * 4 + msg_catch->tamanio_nombre;
		stream = malloc(paquete->buffer->size);

		memcpy(stream + offset, &(msg_catch->tamanio_nombre), sizeof(uint32_t));
		offset += sizeof(uint32_t);
		memcpy(stream + offset, msg_catch->nombre_pokemon, msg_catch->tamanio_nombre);
		offset += msg_catch->tamanio_nombre;
		memcpy(stream + offset, &(msg_catch->coordenada_X), sizeof(uint32_t));
		offset += sizeof(uint32_t);
		memcpy(stream + offset, &(msg_catch->coordenada_Y), sizeof(uint32_t));

		break;

	case CAUGHT_POKEMON:;
		caught_pokemon_msg* msg_caught = (caught_pokemon_msg*) estructura_mensaje;

		paquete->buffer->size = sizeof(uint32_t) * 2;
		stream = malloc(paquete->buffer->size);

		memcpy(stream + offset, &(msg_caught->id_correlativo), sizeof(uint32_t));
		offset += sizeof(uint32_t);
		memcpy(stream + offset, &(msg_caught->resultado), sizeof(uint32_t));

		break;

	default:
		break;
	}

	// El tamaño total sería: id_cola (4) + buffer_size (4) + stream (buffer_size)
	int total_bytes = paquete->buffer->size + sizeof(queue_name) + sizeof(uint32_t);

	paquete->buffer->stream = stream;
	void* a_enviar = serializar_paquete(paquete, total_bytes);

	if (send(socket_receptor, a_enviar, total_bytes, 0) == -1) {
		perror("Error enviando mensaje");
		control = -1;
	}

	send(socket_receptor, &id, sizeof(uint32_t), 0);

	free(paquete->buffer->stream);
	free(paquete->buffer);
	free(paquete);
	free(a_enviar);

	return control;
}


void recorrer_cola(t_cola_de_mensajes* nombre) {

	if (!queue_is_empty(nombre->cola) && (list_size(nombre->lista_suscriptores) != 0)) {

		t_info_mensaje* info_a_sacar = queue_peek(nombre->cola);
		uint32_t id_primero = info_a_sacar->id;
		uint32_t id_siguiente;

		bool sub_suscrito = true;

		do {
			info_a_sacar = queue_pop(nombre->cola);

			for (int i = 0; i < list_size(nombre->lista_suscriptores); i++) {
				uint32_t* sub = (uint32_t*) list_get(nombre->lista_suscriptores,i);
				if (!esta_en_lista(info_a_sacar->quienes_lo_recibieron, sub)) {
					if (mandar(nombre->tipo_cola, de_id_mensaje_a_mensaje(info_a_sacar->id), info_a_sacar->id , *sub) == -1) {

						sub_suscrito = false;

						bool es_igual_a(void* uno) {
							uint32_t nro = *(uint32_t*) uno;
							return nro == *sub;
						}

						list_remove_and_destroy_by_condition(nombre->lista_suscriptores,es_igual_a,free);

						uint32_t id_afuera = info_a_sacar->id;
						do {
							list_remove_and_destroy_by_condition(info_a_sacar->a_quienes_fue_enviado,es_igual_a,free);
							list_remove_and_destroy_by_condition(info_a_sacar->quienes_lo_recibieron,es_igual_a,free);

							queue_push(nombre->cola, info_a_sacar);
							info_a_sacar = queue_pop(nombre->cola);
							id_siguiente = info_a_sacar->id;

						} while (id_afuera != id_siguiente);

					}
					if (!esta_en_lista(info_a_sacar->a_quienes_fue_enviado, sub) && sub_suscrito) {
						list_add(info_a_sacar->a_quienes_fue_enviado, sub);
					}
				}
			}

			queue_push(nombre->cola, info_a_sacar);
			info_a_sacar = queue_peek(nombre->cola);
			id_siguiente = info_a_sacar->id;

		} while (id_primero != id_siguiente);

	}

}

bool esta_en_lista(t_list* a_los_que_envie, uint32_t* sub) {

	bool es_igual(void* uno) {
		uint32_t nro = *(uint32_t*) uno;
		return nro == *sub;
	}
	return list_any_satisfy(a_los_que_envie, es_igual);
}


