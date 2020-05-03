#ifndef RECIBIR_H_
#define RECIBIR_H_

#include "broker.h"

void recibir_mensajes(){
	int id_cola;
	t_paquete* paq = malloc(sizeof(t_paquete));

	while(1){
	recv(socket, &(paq->cola_msg), sizeof(queue_name), MSG_WAITALL);

	id_cola = paq->cola_msg;

	if (paq->buffer->size /= 0){
		paq->buffer = malloc(sizeof(t_buffer));
		recv(socket, &(paq->buffer->size), sizeof(queue_name), MSG_WAITALL);

		paq->buffer->stream = malloc(paq->buffer->size);
		recv(socket, paq->buffer->stream, paq->buffer->size, MSG_WAITALL);

		if (revisar_mensaje(id_cola,paq->buffer)){
			int id_mensaje = crear_nuevo_id();

			enviar_a_publisher_id(id_mensaje);


			agregar_a_cola(id_cola,paq->buffer);
		}
	} else {
		int id_correlativo = paq->buffer->stream;
		confirmar_mensaje(id_cola , id_correlativo);
	}
	}
}

int crear_nuevo_id(){
// agregar semaforo
	contador_id ++;
	return contador id;

}

void agregar_a_cola(int id_cola,t_paquete paquete){

	void* msg = &paquete;

	switch (id_cola){
	case 1:
		queue_push(QUEUE_NEW_POKEMON.cola, msg);
		break;
	case 2:
		queue_push(QUEUE_APPEARED_POKEMON.cola, msg);
		break;
	case 3:
		queue_push(QUEUE_CATCH_POKEMON.cola, msg);
		break;
	case 4:
		queue_push(QUEUE_CAUGHT_POKEMON.cola, msg);
		break;
	case 5:
		queue_push(QUEUE_GET_POKEMON.cola, msg);
		break;
	case 6:
		queue_push(QUEUE_LOCALIZED_POKEMON.cola, msg);
		break;
	}
}


#endif /* RECIBIR_H_ */
