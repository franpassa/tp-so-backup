#include "broker.h"

void inicializar_memoria(){

	*memoria = malloc(atoi(config_get_string_value(config,"TAMANIO_MEMORIA")));
	structura = malloc(sizeof(t_struct_secundaria));
	struct_secundaria = list_create();
	cont_orden = 0;

	if(string_equals_ignore_case(config_get_string_value(config,"ALGORITMO_MEMORIA"),"PARTICIONES")){

		structura->tipo_mensaje = 6;
		structura->tamanio = atoi(config_get_string_value(config,"TAMANIO_MEMORIA"));
		structura->id = 0;
		structura->bit_inicio = 0;

		list_add(struct_secundaria,structura);
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
		for (int i = 0; i< list_size(struct_secundaria); i++ ){
			structura = list_get(struct_secundaria,i);
			if (structura->tipo_mensaje == 6){
				flag ++;
				if(structura->tamanio >= size ){
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

	}else if(string_equals_ignore_case(config_get_string_value(config,"ALGORITMO_PARTICION_LIBRE"),"BF")){
		int sobra;
		int sobra_menor = atoi(config_get_string_value(config,"TAMANIO_MEMORIA"));
		for (int i = 0; i< list_size(struct_secundaria); i++ ){
			structura = list_get(struct_secundaria,i);
			if (structura->tipo_mensaje == 6){
				flag ++;
				if(structura->tamanio >= size ){
					sobra = (structura->tamanio - size);
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
		}else {
			return entra;
		}

	}else{
		printf("Error en broker.config ALGORITMO_PARTICION_LIBRE no valido");
	}
}

int paso_2(int size){
	t_struct_secundaria* structura2 = malloc(sizeof(t_struct_secundaria));
	int tamanio_lista_actal = list_size(struct_secundaria);
	for (int i = 0; i< tamanio_lista_actal; i++ ){
		structura = list_get(struct_secundaria,i);
		if (structura->tipo_mensaje == 6 && i<list_size(struct_secundaria)){
			structura2 = list_get(struct_secundaria,(i+1));
			if(structura2->tipo_mensaje == 6){
				structura->tamanio += structura2->tamanio;

				list_remove_and_destroy_element(struct_secundaria,(i+1),free);
				list_remove_and_destroy_element(struct_secundaria,(i),free);
				list_add_in_index(struct_secundaria,i,structura);

				tamanio_lista_actal -= 1;
				i -= 1;
			}else{

				for (int f = 1; f< (tamanio_lista_actal - i); f++ ){
					t_struct_secundaria* structura3 = malloc(sizeof(t_struct_secundaria));

					structura3 = list_get(struct_secundaria,(i+f));
					structura3->bit_inicio = structura3->bit_inicio - structura->tamanio;
					list_remove_and_destroy_element(struct_secundaria,(i+f),free);
					list_add_in_index(struct_secundaria,(i+f),structura3);
				}
				char* memoria_a_mover = malloc(atoi(config_get_string_value(config,"TAMANIO_MEMORIA"))-structura2->bit_inicio);
				*memoria_a_mover = strdup(memoria + structura2->bit_inicio);
				*(memoria + structura->bit_inicio) = *memoria_a_mover;

				structura2->bit_inicio = atoi(config_get_string_value(config,"TAMANIO_MEMORIA")) - structura->tamanio;
				structura2->id = 0;
				structura2->tamanio = structura->tamanio;
				structura2->tipo_mensaje = 6;

				list_add(struct_secundaria,structura2);
				list_remove_and_destroy_element(struct_secundaria,i,free);
				tamanio_lista_actal -= 1;
				 i -= 1;
			}
		}
	}
	return paso_1(size);
}
int paso_3(int size){
	t_struct_secundaria* structura2 = malloc(sizeof(t_struct_secundaria));
	int orden , orden_menor;
	int a_sacar = 0;
	if(string_equals_ignore_case(config_get_string_value(config,"ALGORITMO_REEMPLAZO"),"FIFO")){
		for(int i = 0; i< list_size(struct_secundaria); i++ ){
			structura = list_get(struct_secundaria,i);
			if(i == 0){
				orden = structura->orden;
				a_sacar = 0;
			}
			orden_menor = structura->orden;
			if (orden_menor < orden){
				orden_menor = orden;
				a_sacar = i;
			}
		}
		for (int f = 1; f< (list_size(struct_secundaria) - a_sacar); f++ ){

			structura2 = list_get(struct_secundaria,(a_sacar+f));
			structura2->bit_inicio = structura2->bit_inicio - structura->tamanio;
			list_remove_and_destroy_element(struct_secundaria,(a_sacar+f),free);
			list_add_in_index(struct_secundaria,(a_sacar+f),structura2);
		}

		structura = list_get(struct_secundaria,(a_sacar+1));
		structura2 = list_get(struct_secundaria,a_sacar);
		char* memoria_a_mover = malloc(atoi(config_get_string_value(config,"TAMANIO_MEMORIA"))-structura->bit_inicio);
		*memoria_a_mover = strdup(memoria + structura->bit_inicio);
		*(memoria + structura2->bit_inicio) = *memoria_a_mover;
		list_remove_and_destroy_element(struct_secundaria,a_sacar,free);

	}else if(string_equals_ignore_case(config_get_string_value(config,"ALGORITMO_REEMPLAZO"),"LRU")){
		int contador = 0;
		int flag_2 = 0;
		do{
			if (contador == list_size(struct_secundaria)){
				contador = 0;
			}
			structura = list_get(struct_secundaria,contador);
			if (structura->uso == 0 ){
				a_sacar = contador;
				flag_2 = 1;
			} else {
				structura->uso = 0;
				list_remove_and_destroy_element(struct_secundaria,contador,free);
				list_add_in_index(struct_secundaria,contador,structura);
			}
			contador ++;
		}while(flag_2 == 0);
		for (int f = 1; f< (list_size(struct_secundaria) - a_sacar); f++ ){

				structura2 = list_get(struct_secundaria,(a_sacar+f));
				structura2->bit_inicio = structura2->bit_inicio - structura->tamanio;
				list_remove_and_destroy_element(struct_secundaria,(a_sacar+f),free);
				list_add_in_index(struct_secundaria,(a_sacar+f),structura2);
		}

		structura = list_get(struct_secundaria,(a_sacar+1));
		structura2 = list_get(struct_secundaria,a_sacar);
		char* memoria_a_mover = malloc(atoi(config_get_string_value(config,"TAMANIO_MEMORIA"))-structura->bit_inicio);
		*memoria_a_mover = strdup(memoria + structura->bit_inicio);
		*(memoria + structura2->bit_inicio) = *memoria_a_mover;
		list_remove_and_destroy_element(struct_secundaria,a_sacar,free);

	}else{
	printf("Error en broker.config ALGORITMO_REEMPLAZO no valido");
	}
	return paso_1(size);
}

int cont_orden_f(){
	cont_orden += 1;
	return cont_orden;
}



