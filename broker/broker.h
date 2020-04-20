/*
 * broker.h
 *
 *  Created on: 15 abr. 2020
 *      Author: utnso
 */

#ifndef BROKER_H_
#define BROKER_H_

#include<commons/collections/queue.h>
#include<commons/collections/list.h>
#include <stdint.h>

#define TRUE 0
#define FALSE 1


typedef struct{
	uint32_t id;
	//t_suscriptor enviados;
	//tipo de mensaje
	uint32_t ack;
} t_mensaje;

typedef struct{
	uint32_t id;
	uint32_t id_correlacional;
} t_suscriptor;

typedef struct{
	t_queue* cola;
	t_list* lista_suscriptores;

}t_cola_de_mensajes;


t_cola_de_mensajes inicializar_cola(){
	t_cola_de_mensajes nueva_cola_de_mensajes;
	t_queue* nueva_cola = queue_create();
	nueva_cola_de_mensajes.cola = nueva_cola;
	nueva_cola_de_mensajes.lista_suscriptores = list_create();
	return nueva_cola_de_mensajes;
}




//inicializar_colas()
//recibir mensaje() -- hilo del broker
//enviar_mensaje_a_suscriptores() -- a una cola especifica
//estado_de_colas()

//MEMORIA
//dump_cache()
//compactar_memoria()




#endif /* BROKER_H_ */
