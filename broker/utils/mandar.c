#include "broker.h"

void mandar_mensajes() {

	int cola_actual = 0;

	while (1) {
		if (cola_actual == 6) {

			cola_actual = NEW_POKEMON;
		}
		if (queue_size(int_a_nombre_cola(cola_actual)->cola) != 0) { // sem_trywait(&contenido_cola[cola_actual]) == 0
			pthread_mutex_lock(&(sem_cola[cola_actual]));
			recorrer_cola(int_a_nombre_cola(cola_actual));
			pthread_mutex_unlock(&(sem_cola[cola_actual]));
		}

		cola_actual++;
	}
}

void mandar(t_paquete* paquete, uint32_t id, uint32_t sub) {
	int total_bytes = paquete->buffer->size + sizeof(queue_name) + sizeof(uint32_t);
	void* bytes = serializar_paquete(paquete, total_bytes);
	send(sub, bytes, total_bytes, 0);
	send(sub, &id, sizeof(uint32_t), 0);
}

void recorrer_cola(t_cola_de_mensajes* nombre) {

	if (!queue_is_empty(nombre->cola)) {

		t_list* subs_queue = nombre->lista_suscriptores;

		if (!list_is_empty(subs_queue)) {

			t_info_mensaje* info_a_sacar = queue_peek(nombre->cola);
			uint32_t id_primero = info_a_sacar->id;
			uint32_t id_siguiente;

			do {
				info_a_sacar = queue_pop(nombre->cola);

				for (int i = 0; i < list_size(subs_queue); i++) {
					uint32_t* sub = (uint32_t*) list_get(subs_queue, i);
					if (!esta_en_lista(info_a_sacar->a_quienes_fue_enviado,sub)) {
						mandar(info_a_sacar->paquete, info_a_sacar->id, *sub);
						list_add(info_a_sacar->a_quienes_fue_enviado, sub);
					}
				}
				queue_push(nombre->cola, info_a_sacar);
				info_a_sacar = queue_peek(nombre->cola);
				id_siguiente = info_a_sacar->id;

			} while (id_primero != id_siguiente);

		}

	}
}

bool esta_en_lista(t_list* a_los_que_envie, uint32_t* sub) {

	bool es_igual(void* uno) {
		uint32_t nro = *(uint32_t*) uno;
		return nro == *sub;
	}
	return list_any_satisfy(a_los_que_envie, es_igual);
}




