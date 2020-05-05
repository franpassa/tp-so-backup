#ifndef MANDAR_H_
#define MANDAR_H_

#include "broker.h"

int cola_actual = 0;

void mandar_mensajes(){

	while(1){
		cola_actual++;

		if (cola_actual == 7){

			cola_actual = 1;
		}

		recorrer_cola(int_a_nombre_cola(cola_actual));
	}
}

bool igual_a(void* uno ,int otro){
	int nro = (int) uno;

	return  nro == otro ;

}

bool no_esten_en(t_list* a_los_que_envie, int sub) {

	bool no_es_igual_a(void* alguien){
		return !igual_a(alguien ,sub);
	}

	return list_any_satisfy(a_los_que_envie,no_es_igual_a);
}

void mandar(t_paquete* paquete, int sub){

	int total_bytes = paquete->buffer->size + sizeof(queue_name) + 2 * sizeof(uint32_t);

	send(sub, paquete, total_bytes, 0);


}

void enviar_a(t_paquete* paquete,t_list* sin_enviar){

	void mandar_a_alguien(void* sub){
		mandar(paquete,(int) sub);
	}

	sin_enviar = list_map(sin_enviar, (void*) mandar_a_alguien);

}

void recorrer_cola(t_cola_de_mensajes nombre){

	if (!queue_is_empty(nombre.cola)){

		t_list* subs = list_create();
		list_add_all(subs,nombre.lista_suscriptores);

		t_list* a_los_q_envie = list_create();
		list_add_all(a_los_q_envie,subs);

		t_list* sin_enviar=list_create();
		list_add_all(sin_enviar,subs);

		if (!list_is_empty(subs)){

			t_info_mensaje* info = queue_peek(nombre.cola);

			int id_primero = info->id;

			int id_siguiente;

			id_primero = id_siguiente;

			do{

				info = queue_pop(nombre.cola);

				a_los_q_envie = info->a_quienes_fue_enviado; // ver manejo de listas

				int sub;

				bool a_los_que_envie_nuevo(void* lista_enviados){
					return no_esten_en(a_los_q_envie,sub);
				}

				sin_enviar = list_filter(subs,a_los_que_envie_nuevo);

				enviar_a(info->paquete,sin_enviar);

				queue_push(nombre.cola,info);

				info = queue_peek(nombre.cola);

				id_siguiente =  info->id;

			}while(id_primero != id_siguiente);
		}

	}
}

#endif /* MANDAR_H_ */
