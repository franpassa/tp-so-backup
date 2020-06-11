#include "broker.h"

void inicializar_memoria() {
	tamanio_memoria = atoi(config_get_string_value(config, "TAMANIO_MEMORIA"));
	memoria = malloc(tamanio_memoria);
	estructura = malloc(sizeof(t_struct_secundaria));
	estructuras_secundarias = list_create();
	cont_orden = 0;

	if (string_equals_ignore_case(config_get_string_value(config, "ALGORITMO_MEMORIA"),"PARTICIONES")) {

		estructura->tipo_mensaje = 6;
		estructura->tamanio = tamanio_memoria;
		estructura->id = 0;
		estructura->bit_inicio = 0;

		list_add(estructuras_secundarias, estructura);
	} else if (string_equals_ignore_case(config_get_string_value(config, "ALGORITMO_MEMORIA"), "BS")) {

	} else {
		printf("Error en broker.config ALGORITMO_MEMORIA no valido");
	}
}

void almacenar(void* mensaje,uint32_t id_cola,uint32_t id_mensaje,uint32_t size){
	entra = -1;
	tamanio_a_ocupar = size;

	if(string_equals_ignore_case(config_get_string_value(config,"ALGORITMO_MEMORIA"),"PARTICIONES")){

		paso_1();
		estructura = list_get(estructuras_secundarias,entra);

		if (estructura->tamanio > size){
			estructura->tamanio = estructura->tamanio - size;
			estructura->bit_inicio = estructura->bit_inicio + size;
			list_add_in_index(estructuras_secundarias,(entra + 1),estructura);
		}

		estructura = list_get(estructuras_secundarias,entra);

		if(string_equals_ignore_case(config_get_string_value(config,"ALGORITMO_REEMPLAZO"),"FIFO")){
			cont_orden ++;
			estructura->auxiliar = cont_orden;
		} else if(string_equals_ignore_case(config_get_string_value(config,"ALGORITMO_REEMPLAZO"),"LRU")) {
			estructura->auxiliar = 1;
		} else {
		printf("Error en broker.config ALGORITMO_REEMPLAZO no valido");
		}

		estructura->id = id_mensaje;
		estructura->tamanio = size;
		estructura->tipo_mensaje = id_cola; // la estructura la llena bien

		list_replace_and_destroy_element(estructuras_secundarias, entra, estructura,free); // Chequear esto porque tira algo valgrind aca

		memcpy(memoria + estructura->bit_inicio, mensaje, size); // aca tambien, por debug recibe los parametros bien, pero explota aca

	}else if(string_equals_ignore_case(config_get_string_value(config,"ALGORITMO_MEMORIA"),"BS")){

		}else{
			printf("Error en broker.config ALGORITMO_MEMORIA no valido");
		}
}

void paso_1(){

	flag = 0;
	if(string_equals_ignore_case(config_get_string_value(config,"ALGORITMO_PARTICION_LIBRE"),"FF")){
		for (int i = 0; i < list_size(estructuras_secundarias); i++ ){
			estructura = list_get(estructuras_secundarias,i);
			if (estructura->tipo_mensaje == 6){
				flag ++;
				if(estructura->tamanio >= tamanio_a_ocupar ){
					entra = i;
				}
			}
		}
		if (entra == -1){
			if (flag > 1){
				paso_2();
			}else{
				paso_3();
			}
		}

	} else if(string_equals_ignore_case(config_get_string_value(config,"ALGORITMO_PARTICION_LIBRE"),"BF")) {
		int sobra;
		int sobra_menor = tamanio_memoria;
		for (int i = 0; i< list_size(estructuras_secundarias); i++ ){
			estructura = list_get(estructuras_secundarias,i);
			if (estructura->tipo_mensaje == 6){
				flag ++;
				if(estructura->tamanio >= tamanio_a_ocupar ){
					sobra = (estructura->tamanio - tamanio_a_ocupar);
					if (sobra<sobra_menor){
						sobra_menor = sobra;
						entra = i;
					}
				}
			}
		}
		if (entra == -1){
			if (flag > 1){
				paso_2();
			}else{
				paso_3();
			}
		}

	} else {
		printf("Error en broker.config ALGORITMO_PARTICION_LIBRE no valido");
	}
}

void paso_2(){
	t_struct_secundaria* estructura2 = malloc(sizeof(t_struct_secundaria));
	int tamanio_lista_actual = list_size(estructuras_secundarias);
	for (int i = 0; i < tamanio_lista_actual; i++ ){
		estructura = list_get(estructuras_secundarias,i);
		if (estructura->tipo_mensaje == 6 && i < list_size(estructuras_secundarias)){ // ver esta duda
			estructura2 = list_get(estructuras_secundarias,(i+1));
			if(estructura2->tipo_mensaje == 6){
				estructura->tamanio += estructura2->tamanio;

				list_remove_and_destroy_element(estructuras_secundarias,(i+1),free);
				list_replace_and_destroy_element(estructuras_secundarias,i,estructura,free);

				tamanio_lista_actual -= 1;
				i -= 1;
			} else {

				actualizar_bit_inicio(i);

				mover_memoria(i);

				estructura2->bit_inicio = tamanio_memoria - estructura->tamanio;
				estructura2->id = 0;
				estructura2->tamanio = estructura->tamanio;
				estructura2->tipo_mensaje = 6;

				list_add(estructuras_secundarias,estructura2);
				i -= 1;
			}
		}
	}
	paso_1();
}

void paso_3(){
	int orden , orden_menor;
	int a_sacar = 0;
	if(string_equals_ignore_case(config_get_string_value(config,"ALGORITMO_REEMPLAZO"),"FIFO")){
		for(int i = 0; i< list_size(estructuras_secundarias); i++ ){
			estructura = list_get(estructuras_secundarias,i);
			if(i == 0){
				orden = estructura->auxiliar;
				a_sacar = 0;
			}
			orden_menor = estructura->auxiliar;
			if (orden_menor < orden && estructura->tipo_mensaje != 6){
				orden_menor = orden;
				a_sacar = i;
			}
		}
		actualizar_bit_inicio(a_sacar);

		mover_memoria(a_sacar);

	} else if(string_equals_ignore_case(config_get_string_value(config,"ALGORITMO_REEMPLAZO"),"LRU")) {
		int contador = 0;
		int flag_2 = 0;
		do {
			if (contador == list_size(estructuras_secundarias)){
				contador = 0;
			}
			estructura = list_get(estructuras_secundarias,contador);
			if (estructura->auxiliar == 0 ){
				a_sacar = contador;
				flag_2 = 1;
			} else {
				estructura->auxiliar = 0;
				list_replace_and_destroy_element(estructuras_secundarias,contador,estructura,free);
			}
			contador ++;
		} while(flag_2 == 0);

		actualizar_bit_inicio(a_sacar);

		mover_memoria(a_sacar);

	} else {
	printf("Error en broker.config ALGORITMO_REEMPLAZO no valido");
	}
	paso_1();
}

int cont_orden_f(){
	cont_orden += 1;
	return cont_orden;
}

void actualizar_bit_inicio(int a_sacar){
	t_struct_secundaria* estructura2 = malloc(sizeof(t_struct_secundaria));

	for (int f = a_sacar; f < list_size(estructuras_secundarias); f++) {
		estructura2 = list_get(estructuras_secundarias, f);
		estructura2->bit_inicio = estructura2->bit_inicio - estructura->tamanio;
		list_replace_and_destroy_element(estructuras_secundarias,f,estructura2,free);

	}

}
void mover_memoria(int a_sacar){

	estructura =  list_get(estructuras_secundarias,a_sacar);

	int tamanio_a_mover = tamanio_memoria - (estructura->bit_inicio + estructura->tamanio);
	char* comienzo_a_sacar = memoria + estructura-> bit_inicio;
	// de donde voy a sacar la memoria a mover
	char* de_donde = memoria + estructura->bit_inicio + estructura->tamanio;

	memmove(comienzo_a_sacar,de_donde,tamanio_a_mover);

	list_remove_and_destroy_element(estructuras_secundarias,a_sacar,free);
	list_add(estructuras_secundarias,estructura);
}

void* de_id_mensaje_a_mensaje(uint32_t id_mensaje) {
	t_struct_secundaria* estructura3 = malloc(sizeof(t_struct_secundaria));
	t_struct_secundaria* estructura_a_comparar_de_lista;

	for (int i = 0; i < list_size(estructuras_secundarias); i++) {
		estructura_a_comparar_de_lista = list_get(estructuras_secundarias,i);
		if (estructura_a_comparar_de_lista->id == id_mensaje) {
			estructura3 = estructura_a_comparar_de_lista;
		}
	}
	void* mensaje = malloc(estructura3->tamanio);
	memcpy(mensaje, memoria + estructura3->bit_inicio, estructura3->tamanio);
	return mensaje;

}

uint32_t de_id_mensaje_a_cola(uint32_t id_mensaje) {
	t_struct_secundaria* estructura3 = malloc(sizeof(t_struct_secundaria));
	t_struct_secundaria* estructura_a_comparar_de_lista;

	for (int i = 0; i < list_size(estructuras_secundarias); i++) {
		estructura_a_comparar_de_lista = list_get(estructuras_secundarias,i);
		if (estructura_a_comparar_de_lista->id == id_mensaje) {
			estructura3 = estructura_a_comparar_de_lista;
		}
	}

	return estructura3->tipo_mensaje;

}

uint32_t de_id_mensaje_a_size(uint32_t id_mensaje) {
	t_struct_secundaria* estructura3 = malloc(sizeof(t_struct_secundaria));
	t_struct_secundaria* estructura_a_comparar_de_lista;

	for (int i = 0; i < list_size(estructuras_secundarias); i++) {
		estructura_a_comparar_de_lista = list_get(estructuras_secundarias, i);
		if (estructura_a_comparar_de_lista->id == id_mensaje) {
			estructura3->tamanio = estructura_a_comparar_de_lista->tamanio;
		}
	}

	return estructura3->tamanio;
}
