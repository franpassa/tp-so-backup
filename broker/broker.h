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
	uint32_t mensaje;
	uint32_t a_quienes_fue_enviado;
	uint32_t cuantos_lo_recibieron;
} t_info_mensaje;

typedef struct{
	uint32_t id;
	uint32_t id_correlacional;
} t_suscriptor;

typedef struct{
	t_queue* cola;
	t_list* lista_suscriptores;

}t_cola_de_mensajes;


t_cola_de_mensajes inicializar_cola(t_cola_de_mensajes nombre_cola){

	t_queue* nueva_cola = queue_create();
	nombre_cola.cola = nueva_cola;
	nombre_cola.lista_suscriptores = list_create();
	return nombre_cola;
}

void recibir_mensajes(){

	// iniciar hilo
	//entro un mensaje
	int id_cola = //sacar info de mensaje
	char* mensaje = //sacar info de mensaje
	int id_mensaje = crear_id_mensaje();

	enviar_a_publisher_id(id_mensaje);

	t_info_mensaje nuevo_mensaje;
	nuevo_mensaje.id = id_mensaje;
	nuevo_mensaje.mensaje = mensaje;
	nuevo_mensaje.a_quienes_fue_enviado = NULL;
	nuevo_mensaje.cuantos_lo_recibieron = NULL;

	agregar_a_cola(id_cola,nuevo_mensaje);

}
void enviar_a_publisher_id(int id){

}
void agregar_a_cola (int id_cola, t_info_mensaje mensaje);{
	switch (id_cola)
	{
	case 1:
			queue_push(NEW_POKEMON.cola, mensaje);
	case 2:
			queue_push(APPEARED_POKEMON.cola, mensaje);
	case 3:
			queue_push(CATCH_POKEMON.cola, mensaje);
	case 4:
			queue_push(CAUGHT_POKEMON.cola, mensaje);
	case 5:
			queue_push(GET_POKEMON.cola, mensaje);
	case 6:
			queue_push(LOCALIZED_POKEMON.cola, mensaje);
	}
}


//recibir mensaje() -- hilo del broker casi
//enviar_mensaje_a_suscriptores()
//estado_de_colas()

//MEMORIA
//dump_cache()
//compactar_memoria()




#endif /* BROKER_H_ */
