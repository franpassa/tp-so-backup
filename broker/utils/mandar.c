#include "broker.h"

void mandar_mensajes(){

	int cola_actual = 0;

	while(1){
		cola_actual++;

		if (cola_actual == 6){

			cola_actual = 0;
		}

		if (cont_cola[cola_actual] == 1){

			pthread_mutex_lock(&(sem_cola[cola_actual]));
			recorrer_cola(int_a_nombre_cola(cola_actual));
			pthread_mutex_unlock(&(sem_cola[cola_actual]));
			cont_cola[cola_actual] = 0;
		}
	}
}


bool igual_a(void* uno ,uint32_t otro){

	int nro = (uint32_t) uno;

	return  nro == otro ;

}

bool no_esten_en(t_list* a_los_que_envie, uint32_t sub) {

	bool no_es_igual_a(void* alguien){
		return !igual_a(alguien ,sub);
	}

	return list_any_satisfy(a_los_que_envie,no_es_igual_a);
}

void mandar(t_paquete* paquete, uint32_t sub){

	int total_bytes = paquete->buffer->size + sizeof(queue_name) + 2 * sizeof(uint32_t);

	send(sub, paquete, total_bytes, 0);


}

void enviar_a(t_paquete* paquete,t_list* sin_enviar){

	void mandar_a_alguien(void* sub){
		mandar(paquete,(uint32_t) sub);
	}

	sin_enviar = list_map(sin_enviar, (void*) mandar_a_alguien);

}

void recorrer_cola(t_cola_de_mensajes* nombre){

	if (!queue_is_empty(nombre->cola)){

		t_list* subs = list_create();

		list_add_all(subs,nombre->lista_suscriptores);

		t_list* a_los_q_envie = list_create();

		t_list* sin_enviar = list_create();

		if (!list_is_empty(subs)){

			t_info_mensaje* info = queue_peek(nombre->cola);

			uint32_t id_primero = info->id;

			uint32_t id_siguiente;

			do {

				info = queue_pop(nombre->cola);

				list_add_all(a_los_q_envie,info->a_quienes_fue_enviado);

				uint32_t sub;

				bool a_los_que_envie_nuevo(void* lista_enviados){
					return no_esten_en(a_los_q_envie,sub);
				}

				sin_enviar = list_filter(subs,a_los_que_envie_nuevo);

				enviar_a(info->paquete,sin_enviar);

				queue_push(nombre->cola,info);

				info = queue_peek(nombre->cola);

				id_siguiente = info->id;

			} while(id_primero != id_siguiente);
		}

	}
}




