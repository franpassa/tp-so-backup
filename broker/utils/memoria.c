#include "broker.h"

void inicializar_memoria() {

	tamanio_memoria = config_get_int_value(config,"TAMANIO_MEMORIA");
	memoria = malloc(tamanio_memoria);

	algoritmo_memoria = config_get_string_value(config,"ALGORITMO_MEMORIA");
	algoritmo_remplazo = config_get_string_value(config,"ALGORITMO_REEMPLAZO");
	algoritmo_part_libre = config_get_string_value(config, "ALGORITMO_PARTICION_LIBRE");
	recuencia_compactacion = config_get_int_value(config,"FRECUENCIA_COMPACTACION");

	t_struct_secundaria* particion_inicial = malloc(sizeof(t_struct_secundaria));
	lista_de_particiones = list_create();
	cont_orden = 0;

	particion_inicial->tipo_mensaje = 6;
	particion_inicial->tamanio = tamanio_memoria;
	particion_inicial->id_mensaje = 0;
	particion_inicial->bit_inicio = 0;
	list_add(lista_de_particiones, particion_inicial);

}


void almacenar(void* mensaje, uint32_t id_cola, uint32_t id_mensaje, uint32_t size){

	entra = -1;
	tamanio_a_ocupar = size;

	if(string_equals_ignore_case(algoritmo_memoria,"PARTICIONES")){

		buscar_particion_en_particiones_dinamicas();

		t_struct_secundaria* estructura_memoria = (t_struct_secundaria*) list_get(lista_de_particiones, entra); // en lista
		t_struct_secundaria* est_a_utilizar = duplicar_estructura(estructura_memoria); // copia que tengo afuera

		if (estructura_memoria->tamanio > size){
			estructura_memoria->tamanio = estructura_memoria->tamanio - size;
			estructura_memoria->bit_inicio = estructura_memoria->bit_inicio + size;
			if(list_size(lista_de_particiones) - 1 == entra){ // Hice este cambio por las dudas estoy comparando la posicion con el tam lista , pongo -1 porque el ultimo elemento es siempre +1 mayor
				list_add(lista_de_particiones, estructura_memoria);
			} else {
				list_add_in_index(lista_de_particiones, (entra + 1), estructura_memoria);
			}
		}

		if(string_equals_ignore_case(algoritmo_remplazo,"FIFO")) {
			cont_orden ++;
			est_a_utilizar->auxiliar = cont_orden;
		} else if(string_equals_ignore_case(algoritmo_remplazo,"LRU")) {
			est_a_utilizar->auxiliar = 1;
		} else {
			printf("Error en broker.config ALGORITMO_REEMPLAZO no valido");
		}

		est_a_utilizar->id_mensaje = id_mensaje;
		est_a_utilizar->tamanio = size;
		est_a_utilizar->tipo_mensaje = id_cola;

        list_replace(lista_de_particiones, entra, est_a_utilizar); // El replace_and_destroy_element tira mas errores en valgrind
        memmove(memoria + est_a_utilizar->bit_inicio, mensaje, size);

        printf("Bit De inicio = %d \n", est_a_utilizar->bit_inicio); // Lo hace bien
        log_info(logger, "MENSAJE ALMACENADO EN PARTICION:%d -- BIT DE INICIO:0x%x", entra, est_a_utilizar->bit_inicio); // LOG 6 en HEXA

	} else if(string_equals_ignore_case(algoritmo_memoria,"BS")) {

		buscar_particion_en_bs(); // la lista de particiones va a tener mas de 1 elemento al hacer esto
		t_struct_secundaria* particion_a_llenar_con_msg = (t_struct_secundaria*) list_get(lista_de_particiones,entra);

		particion_a_llenar_con_msg->tamanio = size;
		particion_a_llenar_con_msg->id_mensaje = id_mensaje;
		particion_a_llenar_con_msg->tipo_mensaje = id_cola;

		if(string_equals_ignore_case(algoritmo_remplazo,"FIFO")) {
			cont_orden ++;
			particion_a_llenar_con_msg->auxiliar = cont_orden;
		} else if(string_equals_ignore_case(algoritmo_remplazo,"LRU")) {
			particion_a_llenar_con_msg->auxiliar = 1;
		} else {
			printf("Error en broker.config ALGORITMO_REEMPLAZO no valido");
		}

		list_replace(lista_de_particiones, entra, particion_a_llenar_con_msg);
		memmove(memoria + particion_a_llenar_con_msg->bit_inicio, mensaje, size);

		log_info(logger, "MENSAJE ALMACENADO EN PARTICION:%d -- BIT DE INICIO:0x%x", entra, particion_a_llenar_con_msg->bit_inicio);; // LOG 6 EN HEXA

		} else {
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
	int encontro_particion = 1;
	int tamanio_lista = list_size(lista_de_particiones);
	if (string_equals_ignore_case(algoritmo_part_libre, "FF")) {
		for (int i = 0; i < tamanio_lista; i++) { // list_iterate podemos usar por ahi (despues verlo)
			particion = list_get(lista_de_particiones, i);
			if (particion->tipo_mensaje == 6) {
				while (particion->tamanio >= tamanio_a_ocupar && encontro_particion != 0) { // divide las particiones (64 -- 32 32 -- 16 16 32 ..)
					if (particion->tamanio / 2 >= tamanio_a_ocupar) {
						t_struct_secundaria* particion_nueva = duplicar_estructura(particion);
						particion->tamanio = particion->tamanio / 2;
						particion_nueva->tamanio = particion->tamanio; // ya va a estar divido el tamanio
						particion_nueva->bit_inicio = particion->bit_inicio + particion->tamanio;
						list_add_in_index(lista_de_particiones, i + 1, particion_nueva);
					} else {
						encontro_particion = 0;
						entra = i; // Encuentra la posicion que entra. Sigue recorriendo la lista por si no encuentra la particion adecuada y haya que eliminar y consolidar.
					}
				}
			}
		}
		if (entra == -1) {
			elegir_victima_para_eliminar_mediante_FIFO_o_LRU_bs(); // Como esta en BS entonces elimino y veo si puedo consolidar
		}
	} else if (string_equals_ignore_case(algoritmo_part_libre, "BF")) {
		int sobra;
		int sobra_menor = tamanio_memoria;
		for (int i = 0; i < list_size(lista_de_particiones); i++) {
			particion = list_get(lista_de_particiones, i);
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
		particion = list_get(lista_de_particiones, entra);
		while (particion->tamanio >= tamanio_a_ocupar && encontro_particion != 0) {
			if (particion->tamanio / 2 >= tamanio_a_ocupar) {
				t_struct_secundaria* particion_nueva = duplicar_estructura(particion);
				particion->tamanio = particion->tamanio / 2;
				particion_nueva->tamanio = particion->tamanio;
				particion_nueva->bit_inicio = particion->bit_inicio + particion->tamanio;
				list_add_in_index(lista_de_particiones, entra + 1, particion_nueva);
			} else {
				encontro_particion = 0;
			}
		}

		if (entra == -1) {
			if (flag > 1) {
				elegir_victima_para_eliminar_mediante_FIFO_o_LRU_bs(); // Como esta en BS entonces elimino y consolido
			}
		}
	} else {
		printf("Error en broker.config ALGORITMO_PARTICION_LIBRE no valido");
	}
}

bool son_buddies(t_struct_secundaria* particion_A, t_struct_secundaria* particion_B) {
	int operacion_xorA_B = particion_B->bit_inicio ^ particion_A->tamanio;
	bool xorA_B = particion_A->bit_inicio == operacion_xorA_B;
	return particion_A->tamanio == particion_B->tamanio && xorA_B;
}

bool es_potencia_de_dos(int numero){
	return ((numero!=0) && ((numero & (numero-1)) == 0)); // & = OPERACION LOGICA AND
}

void consolidar_particiones_en_bs(int posicion_a_liberar) { // Consolido cada vez que se libera una particion y sigo consolidando hasta que no pueda mas
	t_struct_secundaria* particion_a_consolidar = list_get(lista_de_particiones, posicion_a_liberar);

	for (int i = 0; i < list_size(lista_de_particiones); i++) {
		t_struct_secundaria* particion_a_comparar_si_es_buddy = list_get(lista_de_particiones, i);
		while(son_buddies(particion_a_consolidar, particion_a_comparar_si_es_buddy)) {
			log_info(logger,
					"ASOCIACION DE PARTICION:%d -- BIT DE INCICIO:0x%x y PARTICION:%d -- BIT DE INCICIO:0x%x ",
					posicion_a_liberar, particion_a_consolidar->bit_inicio, i,
					particion_a_comparar_si_es_buddy->bit_inicio); // LOG 8
			particion_a_comparar_si_es_buddy->tamanio = particion_a_comparar_si_es_buddy->tamanio + particion_a_consolidar->tamanio;
			list_remove_and_destroy_element(lista_de_particiones, posicion_a_liberar, free);
		}
	}
}

void buscar_particion_en_particiones_dinamicas(){
	t_struct_secundaria* nueva_est;

	if(string_equals_ignore_case(algoritmo_part_libre,"FF")){
		for (int i = 0; i < list_size(lista_de_particiones); i++ ){
			nueva_est = list_get(lista_de_particiones, i);
			if (nueva_est->tipo_mensaje == 6){
				if(nueva_est->tamanio >= tamanio_a_ocupar ){
					entra = i;
				}
			}
		}
		if (entra == -1){
			if (flag == recuencia_compactacion){
				compactar();
			}else if (recuencia_compactacion == -1 && flag == -1){
				compactar();
			}else{
				flag += 1;
				elegir_victima_para_eliminar_mediante_FIFO_o_LRU_particiones();
			}
		}

	} else if(string_equals_ignore_case(algoritmo_part_libre,"BF")) {
		int sobra;
		int sobra_menor = tamanio_memoria;
		for (int i = 0; i< list_size(lista_de_particiones); i++ ){
			nueva_est = list_get(lista_de_particiones,i);
			if (nueva_est->tipo_mensaje == 6){
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
			if (flag == recuencia_compactacion){
				compactar();
			}else if (recuencia_compactacion == -1 && flag == -1){
				compactar();
			}else{
				flag += 1;
				elegir_victima_para_eliminar_mediante_FIFO_o_LRU_particiones();
			}
		}

	} else {
		printf("Error en broker.config ALGORITMO_PARTICION_LIBRE no valido");
	}
}

void compactar(){
	t_struct_secundaria* estructura1;
	t_struct_secundaria* estructura2;
	int tamanio_lista_actual = list_size(lista_de_particiones);
	for (int i = 0; i < tamanio_lista_actual; i++ ){
		estructura1 = list_get(lista_de_particiones,i);
		if (estructura1->tipo_mensaje == 6 && i+1 < tamanio_lista_actual){
			estructura2 = list_get(lista_de_particiones,(i+1));
			if(estructura2->tipo_mensaje == 6){
				log_info(logger, "COMPACTACION DE PARTICION:%d -- BIT DE INCICIO:0x%x y PARTICION:%d -- BIT DE INCICIO:0x%x ",
								i, estructura1->bit_inicio, (i+1),
								estructura2->bit_inicio); // LOG 8

				estructura1->tamanio += estructura2->tamanio;

				list_remove_and_destroy_element(lista_de_particiones,(i+1),free);
				list_replace_and_destroy_element(lista_de_particiones,i,estructura1,free);

				tamanio_lista_actual -= 1;
				i -= 1;
			} else {

				actualizar_bit_inicio(i);

				mover_memoria(i);

				estructura2->bit_inicio = tamanio_memoria - estructura1->tamanio;
				estructura2->id_mensaje = 0;
				estructura2->tamanio = estructura1->tamanio;
				estructura2->tipo_mensaje = 6;

				list_add(lista_de_particiones, estructura2);
				i -= 1;
			}
		}
	}
	buscar_particion_en_particiones_dinamicas();
}

void elegir_victima_para_eliminar_mediante_FIFO_o_LRU_particiones() {

	int a_sacar = 0;

	if (string_equals_ignore_case(algoritmo_remplazo,"FIFO")) {
		a_sacar = algoritmo_FIFO();
	} else if (string_equals_ignore_case(algoritmo_remplazo,"LRU")) {
		a_sacar = algoritmo_LRU();
	} else {
		printf("Error en broker.config ALGORITMO_REEMPLAZO no valido");
	}
	t_struct_secundaria* particion_a_sacar = list_get(lista_de_particiones,a_sacar);
	log_info(logger,"ELIMINO PARTICIO:%d -- BIT DE INCICIO:0x%x", a_sacar, particion_a_sacar->bit_inicio); // LOG 7
	particion_a_sacar->id_mensaje = -1;
	particion_a_sacar->tipo_mensaje= 6;

	bool esVacio(void* una_particion){
		t_struct_secundaria* particion_a_comparar =  (t_struct_secundaria*) una_particion;
		return particion_a_comparar->tipo_mensaje == 6;
	}

	if (list_all_satisfy(lista_de_particiones,esVacio)){
		flag = -1;
	}

	list_replace_and_destroy_element(lista_de_particiones,a_sacar,particion_a_sacar,free);
	buscar_particion_en_particiones_dinamicas();
}

void elegir_victima_para_eliminar_mediante_FIFO_o_LRU_bs() {

	int a_sacar = 0;

	if (string_equals_ignore_case(algoritmo_remplazo, "FIFO")) {
		a_sacar = algoritmo_FIFO();
	} else if (string_equals_ignore_case(algoritmo_remplazo, "LRU")) {
		a_sacar = algoritmo_LRU();
	} else {
		printf("Error en broker.config ALGORITMO_REEMPLAZO no valido");
	}

	t_struct_secundaria* particion_a_sacar = list_get(lista_de_particiones, a_sacar);
	log_info(logger,"ELIMINO PARTICION:%d -- BIT DE INCICIO:0x%x", a_sacar, particion_a_sacar->bit_inicio); // LOG 7
	particion_a_sacar->id_mensaje = 0;
	particion_a_sacar->tipo_mensaje = 6;

	while (!es_potencia_de_dos(particion_a_sacar->tamanio)) { // Le sumo 1 hasta ver si es potencia de 2 ya que la estructura >= al size que tenia antes (ejemplo: tamanio = 6 entonces 7 ..8 para ahi)
		particion_a_sacar->tamanio += 1;
	}
	particion_a_sacar->auxiliar = 0;
	list_replace_and_destroy_element(lista_de_particiones, a_sacar, particion_a_sacar, free);
	// quedaria msg_a_sacar -> [6, tamanio_potencia_de_2, 0, bit de inicio, 0] y en memoria si elimino 2do msg => memoria=hola----andas

	consolidar_particiones_en_bs(a_sacar);
	buscar_particion_en_bs();
}



int algoritmo_FIFO(){
	t_struct_secundaria* particion_a_sacar;
	int orden, orden_menor;
	int a_sacar = -1; // Creo q esta bien esto
	for(int i = 0; i< list_size(lista_de_particiones); i++ ){
		particion_a_sacar = list_get(lista_de_particiones,i);
		if(i == 0){
			orden = particion_a_sacar->auxiliar;
			a_sacar = 0;
		}
		orden_menor = particion_a_sacar->auxiliar;
		if (orden_menor < orden && particion_a_sacar->tipo_mensaje != 6){
			orden_menor = orden;
			a_sacar = i;
		}
	}
	return a_sacar;
}

int algoritmo_LRU(){
	t_struct_secundaria* particion_a_sacar;
	int contador = 0;
	int flag_2 = 0;
	int a_sacar = 0 ;
	do {
		if (contador == list_size(lista_de_particiones)){
			contador = 0;
		}
		particion_a_sacar = list_get(lista_de_particiones,contador);
		if (particion_a_sacar->auxiliar == 0 ){
			a_sacar = contador;
			flag_2 = 1;
		} else {
			particion_a_sacar->auxiliar = 0;
			list_replace_and_destroy_element(lista_de_particiones, contador,particion_a_sacar, free);
		}
		contador ++;
	} while(flag_2 == 0);

	return a_sacar;
}

int cont_orden_f(){
	cont_orden += 1;
	return cont_orden;
}

void actualizar_bit_inicio(int a_sacar){ // actualiza el bit de inicio y lo elimina de la lista de particiones a la particion a sacar // Creo que asi esta perfecto
	t_struct_secundaria* particion_siguiente_de_a_sacar; // a sacar + 1
	t_struct_secundaria* particion_a_sacar;

	for (int f = a_sacar + 1; f < list_size(lista_de_particiones); f++) { // Siempre haces que el bit inicio siguiente se mueva para atras
		particion_siguiente_de_a_sacar = list_get(lista_de_particiones, f);
		particion_a_sacar = list_get(lista_de_particiones, f-1); // el primer for va a ser el a_sacar
		particion_siguiente_de_a_sacar->bit_inicio = particion_siguiente_de_a_sacar->bit_inicio - particion_a_sacar->tamanio;
		list_replace_and_destroy_element(lista_de_particiones,f,particion_siguiente_de_a_sacar,free);
	}
}

void mover_memoria(int a_sacar) {
	t_struct_secundaria* particion_a_mover_memoria;
	particion_a_mover_memoria = list_get(lista_de_particiones, a_sacar);
	int tamanio_a_mover = tamanio_memoria - (particion_a_mover_memoria->bit_inicio + particion_a_mover_memoria->tamanio);
	void* comienzo_a_sacar = memoria + particion_a_mover_memoria->bit_inicio;
	void* de_donde = memoria + particion_a_mover_memoria->bit_inicio + particion_a_mover_memoria->tamanio;
	// de donde voy a sacar la memoria a mover
	memmove(comienzo_a_sacar, de_donde, tamanio_a_mover);
	list_remove_and_destroy_element(lista_de_particiones, a_sacar, free);
	list_add(lista_de_particiones, particion_a_mover_memoria);

}

void* de_id_mensaje_a_mensaje(uint32_t id_mensaje){
	t_struct_secundaria* particion_de_donde_voy_a_sacar_el_tipo_de_cola = encontrar_particion_en_base_a_un_id_mensaje(id_mensaje);
	void* mensaje = malloc(particion_de_donde_voy_a_sacar_el_tipo_de_cola->tamanio);
	pthread_mutex_lock(&(semaforo_memoria));
	memcpy(mensaje, memoria + particion_de_donde_voy_a_sacar_el_tipo_de_cola->bit_inicio, particion_de_donde_voy_a_sacar_el_tipo_de_cola->tamanio);
	pthread_mutex_unlock(&(semaforo_memoria));
	return mensaje;
}

uint32_t de_id_mensaje_a_cola(uint32_t id_mensaje){
	t_struct_secundaria* particion_de_donde_voy_a_sacar_el_tipo_de_cola = encontrar_particion_en_base_a_un_id_mensaje(id_mensaje);
	return particion_de_donde_voy_a_sacar_el_tipo_de_cola->tipo_mensaje;
}


uint32_t de_id_mensaje_a_size(uint32_t id_mensaje){
	t_struct_secundaria* particion_de_donde_voy_a_sacar_el_tipo_de_cola = encontrar_particion_en_base_a_un_id_mensaje(id_mensaje);
	return particion_de_donde_voy_a_sacar_el_tipo_de_cola->tamanio;
}

t_struct_secundaria* encontrar_particion_en_base_a_un_id_mensaje(uint32_t id_mensaje){
	t_struct_secundaria* particion_de_donde_voy_a_sacar_el_tipo_de_cola;
	bool mismo_id(void* una_particion){
		t_struct_secundaria* particion_a_comparar =  (t_struct_secundaria*) una_particion;
		return particion_a_comparar->id_mensaje == id_mensaje;
	}
	pthread_mutex_lock(&(semaforo_struct_s));
	particion_de_donde_voy_a_sacar_el_tipo_de_cola = list_find(lista_de_particiones,mismo_id);
	pthread_mutex_unlock(&(semaforo_struct_s));
	return particion_de_donde_voy_a_sacar_el_tipo_de_cola;
}

void dump_de_cache(){
	log_info(logger,"EJECUCION DE DUMP DE CACHE\n"); //LOG 9
	t_struct_secundaria* particion_a_mostrar;
	FILE* dump_file = fopen("/home/utnso/workspace/tp-2020-1c-Cuarenteam/broker/Default/dump_cache","w");
	if(dump_file == NULL){
		printf("No se pudo abrir el archivo.\n");
		exit(1);
	} else {
	time_t fecha;
	struct tm *info;
	time(&fecha);
	info = localtime(&fecha);
	fprintf(dump_file, "Date:%s ", asctime(info)); // Esta impreso distinto, imprime el dia y el mes en string

	for(int i=0; i < list_size(lista_de_particiones);i++){
		particion_a_mostrar = list_get(lista_de_particiones,i);
		fprintf(dump_file,"Particion %d:", i);
		fprintf(dump_file,"0x%x - 0x%x.\t ",particion_a_mostrar->bit_inicio, particion_a_mostrar->bit_inicio + particion_a_mostrar->tamanio);
		if(particion_a_mostrar->tipo_mensaje == 6){
			fprintf(dump_file,"[L]\t Size:%d b \n", particion_a_mostrar->tamanio);
		} else {
			//VER LRU Depende del algoritmo, igual esta puesto.
			fprintf(dump_file,"[X]\t Size:%d b \t LRU:%d \t Cola:%d \t Id:%d \n", particion_a_mostrar->tamanio,
					particion_a_mostrar->auxiliar, particion_a_mostrar->tipo_mensaje, particion_a_mostrar->id_mensaje);
		}
	}
	}
	fclose(dump_file);
}

void capturar_senial(){
	while(1){
		signal(SIGUSR1, dump_de_cache);
	}
}
