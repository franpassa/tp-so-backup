#include "broker.h"

void mandar_mensajes() {

	while (1) {
		sem_wait(&binario_mandar);
		pthread_mutex_lock(&(semaforo_struct_s));
		recorrer_struct_s();
		pthread_mutex_unlock(&(semaforo_struct_s));
	}
}

int mandar(queue_name cola, void* stream, int id, int socket_receptor, int size ,uint32_t id_correlativo) {
	int control = 0;
	t_paquete* paquete = malloc(sizeof(t_paquete));

	paquete->cola_msg = cola;
	paquete->buffer = malloc(sizeof(t_buffer));


	if (cola == 1 || cola == 3 || cola == 5 ){

		void* stream_a_mandar = malloc(size + sizeof(uint32_t));
		memcpy(stream_a_mandar,&id_correlativo,sizeof(uint32_t));
		memcpy(stream_a_mandar + sizeof(uint32_t), stream, size);
		paquete->buffer->size = size + sizeof(uint32_t);
		paquete->buffer->stream = stream_a_mandar;
	} else {
		paquete->buffer->stream = stream;
		paquete->buffer->size = size;
	}
	// El tamaño total sería: id_cola (4) + buffer_size (4) + stream (buffer_size)
	int total_bytes = paquete->buffer->size + sizeof(queue_name) + sizeof(uint32_t);


	void* a_enviar = serializar_paquete(paquete, total_bytes);

	if (send(socket_receptor, a_enviar, total_bytes, 0) == -1) {
		perror("Error enviando mensaje");
		control = -1;
	}

	send(socket_receptor, &id, sizeof(uint32_t), 0);

	send(socket_receptor, &socket_receptor, sizeof(uint32_t), 0);

	log_info(logger, "MENSAJE CON ID:%d -- ENVIADO A SUSCRIPTOR:%d ", id, socket_receptor); // LOG 5

	free(paquete->buffer);
	free(paquete);
	free(a_enviar);
	return control;

}

void recorrer_struct_s(){
	bool sub_suscrito = true;
	for ( int i=0 ; i < list_size(lista_de_particiones); i++){

		t_struct_secundaria* particion = (t_struct_secundaria*) list_get(lista_de_particiones, i);
		uint32_t tipo_msg = particion->tipo_mensaje;

		t_cola_de_mensajes* cola_mensajes;
		t_list* lista_suscriptores;

		if(tipo_msg >= 0 && tipo_msg < 6){
			cola_mensajes = int_a_nombre_cola(tipo_msg);
			lista_suscriptores = cola_mensajes->lista_suscriptores;
		} else {
			continue;
		}

		for(int j = 0; j < list_size(lista_suscriptores); j++){


			uint32_t* suscriptor = list_get(lista_suscriptores, j);

			uint32_t* sub = malloc(sizeof(uint32_t));
			memcpy(sub, (void*) suscriptor, sizeof(uint32_t));


			if (list_size(int_a_nombre_cola(particion->tipo_mensaje)->lista_suscriptores) != 0){

				if (!esta_en_lista(particion->quienes_lo_recibieron, sub)) {

					void* mensaje = sacar_mensaje_de_memoria(particion->bit_inicio, particion->tamanio);
					bool es_igual_a(void* uno) {
						uint32_t nro = *(uint32_t*) uno;
						return nro == *sub;
					}

					if (!list_any_satisfy(particion->a_quienes_fue_enviado, es_igual_a)){
						if (mandar(particion->tipo_mensaje, mensaje, particion->id_mensaje, *sub, particion->tamanio, particion->id_correlativo) == -1){

							sub_suscrito = false;

							for (int a = 0; a < list_size(lista_de_particiones); a++) {
								t_struct_secundaria* particion_n = list_get(lista_de_particiones, a);
								if(particion_n->tipo_mensaje != 6){
									list_remove_by_condition(particion_n->a_quienes_fue_enviado, es_igual_a);
									list_remove_by_condition(particion_n->quienes_lo_recibieron, es_igual_a);
								}

							}

							list_remove_and_destroy_by_condition(int_a_nombre_cola(particion->tipo_mensaje)->lista_suscriptores,es_igual_a, free);
							free(sub);
						}
					}
					free(mensaje);
					if(string_equals_ignore_case(algoritmo_remplazo,"LRU")){
						printf("Actualizo bit de particion=%d\n",i);
						particion->auxiliar = f_cont_lru();
				    }

					if (!esta_en_lista(particion->a_quienes_fue_enviado, sub) && sub_suscrito) {
						list_add(particion->a_quienes_fue_enviado, sub);

					}
				}

			}

		}
	}

}

bool esta_en_lista(t_list* a_los_que_envie, uint32_t* sub) {
	if(list_is_empty(a_los_que_envie)){
		return 0;
	}

	bool es_igual(void* uno) {
		uint32_t nro = *(uint32_t*) uno;
		return nro == *sub;
	}
	return list_any_satisfy(a_los_que_envie, es_igual);
}

