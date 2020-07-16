#include "broker.h"

void mandar_mensajes() {

	uint32_t cola_actual = 0;

	while (1) {
		if (cola_actual == 6) {
			cola_actual = 0;
		}
		t_cola_de_mensajes* cola_a_revisar = int_a_nombre_cola(cola_actual);
		if (!queue_is_empty(cola_a_revisar->cola)){
			pthread_mutex_lock(&(sem_cola[cola_actual]));
			recorrer_cola(int_a_nombre_cola(cola_actual));
			pthread_mutex_unlock(&(sem_cola[cola_actual]));
		}

		cola_actual++;
	}
}

int mandar(queue_name cola, void* stream, int id, int socket_receptor, int size) {
	int control = 0;
	t_paquete* paquete = malloc(sizeof(t_paquete));

	paquete->cola_msg = cola;
	paquete->buffer = malloc(sizeof(t_buffer));
	paquete->buffer->size = size;

	// El tamaño total sería: id_cola (4) + buffer_size (4) + stream (buffer_size)
	int total_bytes = paquete->buffer->size + sizeof(queue_name) + sizeof(uint32_t);

	paquete->buffer->stream = stream;
	void* a_enviar = serializar_paquete(paquete, total_bytes);

	if (send(socket_receptor, a_enviar, total_bytes, 0) == -1) {
		perror("Error enviando mensaje");
		control = -1;
	}

	send(socket_receptor, &id, sizeof(uint32_t), 0);

	//log_info(logger, " MENSAJE:%s -- ID:%d -- ENVIADO A SUSCRIPTOR: %d ", msg_as_string(cola, stream), id, socket_receptor); // LOG 4 tira sigsev en gameboy

	free_paquete(paquete);
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

					void* mensaje = de_id_mensaje_a_mensaje(info_a_sacar->id);

					if (mandar(nombre->tipo_cola, mensaje, info_a_sacar->id, *sub, de_id_mensaje_a_size(info_a_sacar->id)) == -1) {

						sub_suscrito = false;

						bool es_igual_a(void* uno) {
							uint32_t nro = *(uint32_t*) uno;
							return nro == *sub;
						}

						list_remove_and_destroy_by_condition(nombre->lista_suscriptores, es_igual_a, free);

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

