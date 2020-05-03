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


			agregar_a_cola(id_cola,paq);
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
	return contador_id;

}

void agregar_a_cola(int id_cola,t_paquete paquete){

	void* msg = &paquete;

	queue_push(int_a_nombre_cola(id_cola).cola, msg);

}


#endif /* RECIBIR_H_ */
