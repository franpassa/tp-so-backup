#include "broker.h"

void inicializar_memoria(){
	tamanio_memoria = atoi(config_get_string_value(config,"TAMANIO_MEMORIA"));
	memoria = malloc(tamanio_memoria);
	estructura = malloc(sizeof(t_struct_secundaria));
	estructura_secundaria = list_create();
	cont_orden = 0;

	if(string_equals_ignore_case(config_get_string_value(config,"ALGORITMO_MEMORIA"),"PARTICIONES")){

		estructura->tipo_mensaje = 6;
		estructura->tamanio = tamanio_memoria;
		estructura->id = 0;
		estructura->bit_inicio = 0;

		list_add(estructura_secundaria,estructura);
	}else if(string_equals_ignore_case(config_get_string_value(config,"ALGORITMO_MEMORIA"),"BS")){

	}else{
		printf("Error en broker.config ALGORITMO_MEMORIA no valido");
	}
}

void almacenar(void* mensaje,int id_cola,int id_mensaje,int size){

	flag = 0;
	entra = -1;

	if(string_equals_ignore_case(config_get_string_value(config,"ALGORITMO_MEMORIA"),"PARTICIONES")){


	}else if(string_equals_ignore_case(config_get_string_value(config,"ALGORITMO_MEMORIA"),"BS")){

		}else{
			printf("Error en broker.config ALGORITMO_MEMORIA no valido");
		}
}

int paso_1(int size){

	if(string_equals_ignore_case(config_get_string_value(config,"ALGORITMO_PARTICION_LIBRE"),"FF")){
		for (int i = 0; i < list_size(estructura_secundaria); i++ ){
			estructura = list_get(estructura_secundaria,i);
			if (estructura->tipo_mensaje == 6){
				flag ++;
				if(estructura->tamanio >= size ){
					entra = i;
					return entra;
				}
			}
		}
		if (entra == -1){
			if (flag > 1){
				return paso_2(size);
			}else{
				return paso_3(size);
			}
		}

	} else if(string_equals_ignore_case(config_get_string_value(config,"ALGORITMO_PARTICION_LIBRE"),"BF")) {
		int sobra;
		int sobra_menor = tamanio_memoria;
		for (int i = 0; i< list_size(estructura_secundaria); i++ ){
			estructura = list_get(estructura_secundaria,i);
			if (estructura->tipo_mensaje == 6){
				flag ++;
				if(estructura->tamanio >= size ){
					sobra = (estructura->tamanio - size);
					if (sobra<sobra_menor){
						sobra_menor = sobra;
						entra = i;
					}
				}
			}
		}
		if (entra == -1){
			if (flag > 1){
				return paso_2(size);
			}else{
				return paso_3(size);
			}
		} else {
			return entra;
		}

	} else {
		printf("Error en broker.config ALGORITMO_PARTICION_LIBRE no valido");
	}
	//falta retornar algo aca
}

int paso_2(int size){
	t_struct_secundaria* estructura2 = malloc(sizeof(t_struct_secundaria));
	int tamanio_lista_actual = list_size(estructura_secundaria);
	for (int i = 0; i < tamanio_lista_actual; i++ ){
		estructura = list_get(estructura_secundaria,i);
		if (estructura->tipo_mensaje == 6 && i < list_size(estructura_secundaria)){
			estructura2 = list_get(estructura_secundaria,(i+1));
			if(estructura2->tipo_mensaje == 6){
				estructura->tamanio += estructura2->tamanio;

				list_remove_and_destroy_element(estructura_secundaria,(i+1),free);
				list_remove_and_destroy_element(estructura_secundaria,(i),free);
				list_add_in_index(estructura_secundaria,i,estructura);

				tamanio_lista_actual -= 1;
				i -= 1;
			}else{

				for (int f = 1; f < (tamanio_lista_actual - i); f++ ){
					t_struct_secundaria* structura3 = malloc(sizeof(t_struct_secundaria));

					structura3 = list_get(estructura_secundaria,(i+f));
					structura3->bit_inicio = structura3->bit_inicio - estructura->tamanio;
					list_remove_and_destroy_element(estructura_secundaria,(i+f),free);
					list_add_in_index(estructura_secundaria,(i+f),structura3);
				}
				char* memoria_a_mover = malloc(tamanio_memoria - estructura2->bit_inicio);
				memoria_a_mover = strdup(memoria + estructura2->bit_inicio);
				*(memoria + estructura->bit_inicio) = *memoria_a_mover;

				estructura2->bit_inicio = tamanio_memoria - estructura->tamanio;
				estructura2->id = 0;
				estructura2->tamanio = estructura->tamanio;
				estructura2->tipo_mensaje = 6;

				list_add(estructura_secundaria,estructura2);
				list_remove_and_destroy_element(estructura_secundaria,i,free);
				tamanio_lista_actual -= 1;
				i -= 1;
			}
		}
	}
	return paso_1(size);
}
int paso_3(int size){
	t_struct_secundaria* estructura2 = malloc(sizeof(t_struct_secundaria));
	int orden , orden_menor;
	int a_sacar = 0;
	if(string_equals_ignore_case(config_get_string_value(config,"ALGORITMO_REEMPLAZO"),"FIFO")){
		for(int i = 0; i< list_size(estructura_secundaria); i++ ){
			estructura = list_get(estructura_secundaria,i);
			if(i == 0){
				orden = estructura->orden;
				a_sacar = 0;
			}
			orden_menor = estructura->orden;
			if (orden_menor < orden){
				orden_menor = orden;
				a_sacar = i;
			}
		}
		for (int f = 1; f < (list_size(estructura_secundaria) - a_sacar); f++ ){

			estructura2 = list_get(estructura_secundaria,(a_sacar+f));
			estructura2->bit_inicio = estructura2->bit_inicio - estructura->tamanio;
			list_remove_and_destroy_element(estructura_secundaria,(a_sacar+f),free);
			list_add_in_index(estructura_secundaria,(a_sacar+f),estructura2);
		}

		estructura = list_get(estructura_secundaria,(a_sacar+1));
		estructura2 = list_get(estructura_secundaria,a_sacar);
		char* memoria_a_mover = malloc(tamanio_memoria - estructura->bit_inicio);
		memoria_a_mover = strdup(memoria + estructura->bit_inicio);
		*(memoria + estructura2->bit_inicio) = *memoria_a_mover;
		list_remove_and_destroy_element(estructura_secundaria,a_sacar,free);

	} else if(string_equals_ignore_case(config_get_string_value(config,"ALGORITMO_REEMPLAZO"),"LRU")) {
		int contador = 0;
		int flag_2 = 0;
		do {
			if (contador == list_size(estructura_secundaria)){
				contador = 0;
			}
			estructura = list_get(estructura_secundaria,contador);
			if (estructura->uso == 0 ){
				a_sacar = contador;
				flag_2 = 1;
			} else {
				estructura->uso = 0;
				list_remove_and_destroy_element(estructura_secundaria,contador,free);
				list_add_in_index(estructura_secundaria,contador,estructura);
			}
			contador ++;
		} while(flag_2 == 0);

		for (int f = 1; f < (list_size(estructura_secundaria) - a_sacar); f++) {
			estructura2 = list_get(estructura_secundaria, (a_sacar + f));
			estructura2->bit_inicio = estructura2->bit_inicio - estructura->tamanio;
			list_remove_and_destroy_element(estructura_secundaria, (a_sacar + f), free);
			list_add_in_index(estructura_secundaria, (a_sacar + f), estructura2);
		}

		estructura = list_get(estructura_secundaria,(a_sacar+1));
		estructura2 = list_get(estructura_secundaria,a_sacar);
		char* memoria_a_mover = malloc(tamanio_memoria - estructura->bit_inicio);
		memoria_a_mover = strdup(memoria + estructura->bit_inicio);
		*(memoria + estructura2->bit_inicio) = *memoria_a_mover;
		list_remove_and_destroy_element(estructura_secundaria,a_sacar,free);

	} else {
	printf("Error en broker.config ALGORITMO_REEMPLAZO no valido");
	}
	return paso_1(size);
}

int cont_orden_f(){
	cont_orden += 1;
	return cont_orden;
}



