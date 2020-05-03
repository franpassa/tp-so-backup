/*
 * mandar.h
 *
 *  Created on: 3 may. 2020
 *      Author: utnso
 */

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

bool igual_a(int uno ,int otro){
	return uno == otro;
}

bool no_esten_en(t_list a_los_que_envie,int sub){
	int alguien;
	return !list_any_satisfy(a_los_que_envie,igual_a(sub,alguien));
}
void mandar(t_paquete paquete, int sub){
	int total_bytes = paquete->buffer->size + sizeof(queue_name) + 2 * sizeof(uint32_t);
	send(sub, paquete, total_bytes, 0);
}
void enviar_a(t_paquete paquete,t_list sin_enviar){
	int sub;
	sin_enviar = list_map(sin_enviar, mandar(paquete,sub));
}
void recorrer_cola(t_cola_de_mensajes nombre){
	if (!queue_is_empty(nombre.cola)){
		t_list subs = nombre.lista_suscriptores, a_los_q_envie,sin_enviar;
		if (!list_is_empty(subs)){
			t_info_mensaje info = queue_peek(nombre.cola);
			int id_primero =  info.id,id_siguiente;

			do{
				info = queue_pop(nombre.cola);
				a_los_q_envie = info.a_quienes_fue_enviado;
				int sub;
				sin_enviar = list_filter(subs,no_esten_en(a_los_q_envie,sub));
				enviar_a(info.paquete,sin_enviar);
				queue_push(nombre.cola,info);
				info = queue_peek(nombre.cola);
				id_siguiente =  info.id;
			}while(id_primero != id_siguiente);
		}

	}
}

#endif /* MANDAR_H_ */