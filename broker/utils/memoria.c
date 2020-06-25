#include "broker.h"

void inicializar_memoria() {

	tamanio_memoria = config_get_int_value(config,"TAMANIO_MEMORIA");
	memoria = malloc(tamanio_memoria);

	estructura = malloc(sizeof(t_struct_secundaria));
	estructuras_secundarias = list_create();
	cont_orden = 0;

	if (string_equals_ignore_case(config_get_string_value(config,"ALGORITMO_MEMORIA"),"PARTICIONES")) {

		estructura->tipo_mensaje = 6;
		estructura->tamanio = tamanio_memoria;
		estructura->id_mensaje = 0;
		estructura->bit_inicio = 0;
		list_add(estructuras_secundarias,estructura);

	} else if (string_equals_ignore_case(config_get_string_value(config,"ALGORITMO_MEMORIA"), "BS")) {

	} else {
		printf("Error en broker.config ALGORITMO_MEMORIA no valido");
	}
}


void almacenar(void* mensaje, uint32_t id_cola, uint32_t id_mensaje, uint32_t size){

	entra = -1;
	tamanio_a_ocupar = size;

	if(string_equals_ignore_case(config_get_string_value(config,"ALGORITMO_MEMORIA"),"PARTICIONES")){

		paso_1();

		t_struct_secundaria* estructura_memoria = (t_struct_secundaria*) list_get(estructuras_secundarias,entra); // en lista
		t_struct_secundaria* est_a_utilizar = duplicar_estructura(estructura_memoria); // copia que tengo afuera

		if (estructura_memoria->tamanio > size){
			estructura_memoria->tamanio = estructura_memoria->tamanio - size;
			estructura_memoria->bit_inicio = estructura_memoria->bit_inicio + size; // ver esto no esta tan chequeado
			list_add_in_index(estructuras_secundarias, (entra + 1), estructura_memoria);
		}

		if(string_equals_ignore_case(config_get_string_value(config,"ALGORITMO_REEMPLAZO"),"FIFO")) {
			cont_orden ++;
			est_a_utilizar->auxiliar = cont_orden;
		} else if(string_equals_ignore_case(config_get_string_value(config,"ALGORITMO_REEMPLAZO"),"LRU")) {
			est_a_utilizar->auxiliar = 1;
		} else {
			printf("Error en broker.config ALGORITMO_REEMPLAZO no valido");
		}

		est_a_utilizar->id_mensaje = id_mensaje;
		est_a_utilizar->tamanio = size;
		est_a_utilizar->tipo_mensaje = id_cola;
		est_a_utilizar->bit_inicio = est_a_utilizar->bit_inicio + size; // Estoy hay que chequearlo

        list_replace(estructuras_secundarias, entra, est_a_utilizar);
        memmove(memoria + est_a_utilizar->bit_inicio, mensaje, size); // esto tambien (nunca empezas desde la posicion 0 de la memoria)


	} else if(string_equals_ignore_case(config_get_string_value(config,"ALGORITMO_MEMORIA"),"BS")) {

		buscar_particion_en_bs();
		// Idem que en particiones pero quiero verificarlo que hay cosas que no me cierran  :)

		}else{
			printf("Error en broker.config ALGORITMO_MEMORIA no valido");
		}
}

t_struct_secundaria* duplicar_estructura(t_struct_secundaria* estructura){
	t_struct_secundaria* duplicado = malloc(sizeof(t_struct_secundaria));
	duplicado->bit_inicio = estructura->bit_inicio;
	duplicado->auxiliar = estructura->auxiliar;
	duplicado->id_mensaje = estructura->id_mensaje;
	duplicado->tamanio = estructura->tamanio;
	duplicado->tipo_mensaje = estructura->tipo_mensaje;
	return duplicado;
}


void buscar_particion_en_bs() {
	t_struct_secundaria* particion;
	flag = 0;
	int encontro_particion = 1;
	if (string_equals_ignore_case(config_get_string_value(config, "ALGORITMO_PARTICION_LIBRE"),"FF")) {
		for (int i = 0; i < list_size(estructuras_secundarias); i++) {
			particion = list_get(estructuras_secundarias, i);
			if (particion->tipo_mensaje == 6) {
				flag++;
				while (particion->tamanio >= tamanio_a_ocupar && encontro_particion != 0) {
					if (particion->tamanio / 2 > tamanio_a_ocupar) {
						t_struct_secundaria* particion_nueva = duplicar_estructura(particion);
						particion->tamanio = particion->tamanio / 2;
						particion_nueva->tamanio = particion->tamanio / 2;
						list_add_in_index(estructuras_secundarias, i + 1, particion_nueva);
					} else {
						encontro_particion = 0;
						entra = i;
						break; // para que no siga recorriendo el for.
					}
				}
			}
		}
		if (entra == -1) {
			if (flag > 1) {
				consolidar_particiones_en_bs();
			} else {
				paso_3();
			}
		}

	} else if(string_equals_ignore_case(config_get_string_value(config, "ALGORITMO_PARTICION_LIBRE"),"BF")){
		int sobra;
		int sobra_menor = tamanio_memoria;
		for (int i = 0; i < list_size(estructuras_secundarias); i++) {
			particion = list_get(estructuras_secundarias, i);
			if (particion->tipo_mensaje == 6) {
				flag++;
				if (particion->tamanio >= tamanio_a_ocupar) {
					sobra = (particion->tamanio - tamanio_a_ocupar);
					if (sobra < sobra_menor) {
						sobra_menor = sobra;
						entra = i;
					}
				}
			}
		}
		// termina de recorrer la lista, encuentra la particion mediante "BF" y luego hace bs para asignarle el tamanio justo a "entra"
		particion = list_get(estructuras_secundarias, entra);
		while (particion->tamanio >= tamanio_a_ocupar && encontro_particion != 0) {
			if (particion->tamanio / 2 > tamanio_a_ocupar) {
				t_struct_secundaria* particion_nueva = duplicar_estructura(particion);
				particion->tamanio = particion->tamanio / 2;
				particion_nueva->tamanio = particion->tamanio / 2;
				list_add_in_index(estructuras_secundarias, entra + 1, particion_nueva);
			} else {
				encontro_particion = 0;
			}
		}

		if (entra == -1) {
			if (flag > 1) {
				consolidar_particiones_en_bs();
			} else {
				paso_3();
			}
		}
	}
}

void consolidar_particiones_en_bs(){
	t_struct_secundaria* particion;
	t_struct_secundaria* particion_siguiente;
	for (int i = 0; i < list_size(estructuras_secundarias); i+=2) { // recorre cada 2 elementos ya que son buddys
		particion = list_get(estructuras_secundarias, i);
		particion_siguiente = list_get(estructuras_secundarias, i+1);
		if(particion->tipo_mensaje == 6 && particion_siguiente->tipo_mensaje == 6 && particion->tamanio == particion_siguiente->tamanio){
			particion->tamanio += particion_siguiente->tamanio;
			list_remove_and_destroy_element(estructuras_secundarias,(i+1),free); // elimino la siguiente particion y le sumo a la anterior el tamanio de la primera
		}
	}
}

void paso_1(){
	t_struct_secundaria* nueva_est;

	flag = 0;
	if(string_equals_ignore_case(config_get_string_value(config,"ALGORITMO_PARTICION_LIBRE"),"FF")){
		for (int i = 0; i < list_size(estructuras_secundarias); i++ ){
			nueva_est = list_get(estructuras_secundarias,i);
			if (nueva_est->tipo_mensaje == 6){
				flag ++;
				if(nueva_est->tamanio >= tamanio_a_ocupar ){
					entra = i; // Si encuentra mas de un lugar, 0, 2, 3, entonces entra = 3, no deberia ser entra= 0?
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
			nueva_est = list_get(estructuras_secundarias,i);
			if (nueva_est->tipo_mensaje == 6){
				flag ++;
				if(nueva_est->tamanio >= tamanio_a_ocupar ){
					sobra = (nueva_est->tamanio - tamanio_a_ocupar);
					if (sobra < sobra_menor){
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
	t_struct_secundaria* estructura2;
	int tamanio_lista_actual = list_size(estructuras_secundarias);
	for (int i = 0; i < tamanio_lista_actual; i++ ){
		estructura = list_get(estructuras_secundarias,i);
		if (estructura->tipo_mensaje == 6 && i < list_size(estructuras_secundarias)){
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
				estructura2->id_mensaje = 0;
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

	if(string_equals_ignore_case(config_get_string_value(config,"ALGORITMO_MEMORIA"),"PARTCIONES")){
		paso_1();
	} else if(string_equals_ignore_case(config_get_string_value(config,"ALGORITMO_MEMORIA"),"BS")){
		buscar_particion_en_bs();
	} else{
		printf("Error en broker.config ALGORITMO_MEMORIA no valido");
	}
}

int cont_orden_f(){
	cont_orden += 1;
	return cont_orden;
}

void actualizar_bit_inicio(int a_sacar){
	t_struct_secundaria* estructura_bit_inicio;

	for (int f = a_sacar + 1; f < list_size(estructuras_secundarias); f++) {
		estructura_bit_inicio = list_get(estructuras_secundarias, f);
		estructura_bit_inicio->bit_inicio = estructura_bit_inicio->bit_inicio - estructura->tamanio;
		list_replace_and_destroy_element(estructuras_secundarias,f,estructura_bit_inicio,free);
	}
}

void mover_memoria(int a_sacar){
	t_struct_secundaria* estructura_a_mover_memoria;

	estructura_a_mover_memoria = list_get(estructuras_secundarias,a_sacar);
	int tamanio_a_mover = tamanio_memoria - (estructura_a_mover_memoria->bit_inicio + estructura_a_mover_memoria->tamanio);
	char* comienzo_a_sacar = memoria + estructura_a_mover_memoria-> bit_inicio;
	// de donde voy a sacar la memoria a mover
	char* de_donde = memoria + estructura_a_mover_memoria->bit_inicio + estructura_a_mover_memoria->tamanio;

	memmove(comienzo_a_sacar,de_donde,tamanio_a_mover);

	list_remove_and_destroy_element(estructuras_secundarias,a_sacar,free);
	list_add(estructuras_secundarias,estructura_a_mover_memoria);
}

void* de_id_mensaje_a_mensaje(uint32_t id_mensaje) {

	t_struct_secundaria* estructura3 = malloc(sizeof(t_struct_secundaria));
	t_struct_secundaria* estructura_a_comparar_de_lista;

	for (int i = 0; i < list_size(estructuras_secundarias); i++) {
		estructura_a_comparar_de_lista = list_get(estructuras_secundarias,i);
		if (estructura_a_comparar_de_lista->id_mensaje == id_mensaje) {
			estructura3->tamanio = estructura_a_comparar_de_lista->tamanio;
			estructura3->bit_inicio = estructura_a_comparar_de_lista->bit_inicio;
		}
	}
	void* mensaje = malloc(estructura3->tamanio);
	memcpy(mensaje, memoria + estructura3->bit_inicio, estructura3->tamanio); // el bit de inicio nunca se asigna por eso daba cualquier cosa

	free(estructura3);
	return mensaje;
}

uint32_t de_id_mensaje_a_cola(uint32_t id_mensaje) { // Perfecto

	t_struct_secundaria* estructura3 = malloc(sizeof(t_struct_secundaria));
	t_struct_secundaria* estructura_a_comparar_de_lista;

	for (int i = 0; i < list_size(estructuras_secundarias); i++) {
		estructura_a_comparar_de_lista = list_get(estructuras_secundarias,i);
		if (estructura_a_comparar_de_lista->id_mensaje == id_mensaje) {
			estructura3->tipo_mensaje = estructura_a_comparar_de_lista->tipo_mensaje;
		}
	}
	return estructura3->tipo_mensaje;
}


uint32_t de_id_mensaje_a_size(uint32_t id_mensaje) { // Perfecto

	t_struct_secundaria* estructura3 = malloc(sizeof(t_struct_secundaria));
	t_struct_secundaria* estructura_a_comparar_de_lista;

	for (int i = 0; i < list_size(estructuras_secundarias); i++) {
		estructura_a_comparar_de_lista = list_get(estructuras_secundarias, i);
		if (estructura_a_comparar_de_lista->id_mensaje == id_mensaje) {
			estructura3->tamanio = estructura_a_comparar_de_lista->tamanio;
		}
	}

	return estructura3->tamanio;
}


