#include "broker.h"

void inicializar_memoria() {

	tamanio_memoria = config_get_int_value(config,"TAMANIO_MEMORIA");
	memoria = malloc(tamanio_memoria);

	algoritmo_memoria = config_get_string_value(config,"ALGORITMO_MEMORIA");
	algoritmo_remplazo = config_get_string_value(config,"ALGORITMO_REEMPLAZO");
	algoritmo_part_libre = config_get_string_value(config, "ALGORITMO_PARTICION_LIBRE");
	frecuencia_compactacion = config_get_int_value(config,"FRECUENCIA_COMPACTACION");
	tamanio_minimo = config_get_int_value(config,"TAMANIO_MINIMO_PARTICION");

	t_struct_secundaria* particion_inicial = malloc(sizeof(t_struct_secundaria));
	lista_de_particiones = list_create();
	cont_orden = 0;

	particion_inicial->tipo_mensaje = 6;
	particion_inicial->tamanio = tamanio_memoria;
	particion_inicial->id_mensaje = 0;
	particion_inicial->bit_inicio = 0;
	particion_inicial->auxiliar = -1;
	list_add(lista_de_particiones, particion_inicial);
}

void almacenar(void* mensaje, uint32_t id_cola, uint32_t id_mensaje, uint32_t size){
	printf("Almacenar\n");
	entra = -1;
	tamanio_a_ocupar = size;

	if(string_equals_ignore_case(algoritmo_memoria,"PARTICIONES")){

		buscar_particion_en_particiones_dinamicas();
		pthread_mutex_lock(&(semaforo_struct_s));
		t_struct_secundaria* estructura_memoria = (t_struct_secundaria*) list_get(lista_de_particiones, entra); // en lista


		if (estructura_memoria->tamanio > mayor_entre_Min_y_tam(size) && estructura_memoria->tamanio - mayor_entre_Min_y_tam(size) >= tamanio_minimo ){
			t_struct_secundaria* est_nueva = duplicar_estructura(estructura_memoria);
			est_nueva->tamanio = estructura_memoria->tamanio - mayor_entre_Min_y_tam(size);
			est_nueva->bit_inicio = estructura_memoria->bit_inicio + mayor_entre_Min_y_tam(size);
			if(list_size(lista_de_particiones) - 1 == entra){ // Hice este cambio por las dudas estoy comparando la posicion con el tam lista , pongo -1 porque el ultimo elemento es siempre +1 mayor
				list_add(lista_de_particiones, est_nueva);
			} else {
				list_add_in_index(lista_de_particiones, (entra + 1), est_nueva);
			}
		}

		if(string_equals_ignore_case(algoritmo_remplazo,"FIFO")) {
			cont_orden ++;
			estructura_memoria->auxiliar = cont_orden;
		} else if(string_equals_ignore_case(algoritmo_remplazo,"LRU")) {
			//de alguna forma poner la hora
			estructura_memoria->auxiliar = 1;
		} else {
			printf("Error en broker.config ALGORITMO_REEMPLAZO no valido");
		}

		estructura_memoria->id_mensaje = id_mensaje;
		estructura_memoria->tamanio = size;
		estructura_memoria->tipo_mensaje = id_cola;

        pthread_mutex_lock(&(semaforo_memoria));
        memmove(memoria + estructura_memoria->bit_inicio, mensaje, size);
        pthread_mutex_unlock(&(semaforo_memoria));


        printf("Bit De inicio = %d \n", estructura_memoria->bit_inicio); // Lo hace bien
        log_info(logger, "MENSAJE ALMACENADO EN PARTICION:%d -- BIT DE INICIO:%d", entra, estructura_memoria->bit_inicio); // LOG 6 en HEXA

        pthread_mutex_unlock(&(semaforo_struct_s));

	} else if(string_equals_ignore_case(algoritmo_memoria,"BS")) {

		buscar_particion_en_bs(); // la lista de particiones va a tener mas de 1 elemento al hacer esto
		pthread_mutex_lock(&(semaforo_struct_s));
		t_struct_secundaria* particion_a_llenar_con_msg = (t_struct_secundaria*) list_get(lista_de_particiones,entra);


		particion_a_llenar_con_msg->tamanio = size;
		particion_a_llenar_con_msg->id_mensaje = id_mensaje;
		particion_a_llenar_con_msg->tipo_mensaje = id_cola;

		if(string_equals_ignore_case(algoritmo_remplazo,"FIFO")) {
			cont_orden ++;
			particion_a_llenar_con_msg->auxiliar = cont_orden;
		} else if(string_equals_ignore_case(algoritmo_remplazo,"LRU")) {
			//tema de hora
			particion_a_llenar_con_msg->auxiliar = 1;
		} else {
			printf("Error en broker.config ALGORITMO_REEMPLAZO no valido");
		}
		printf("BIT AUXILIAR = %d\n",particion_a_llenar_con_msg->auxiliar);
		pthread_mutex_lock(&(semaforo_memoria));
		memmove(memoria + particion_a_llenar_con_msg->bit_inicio, mensaje, size);
		pthread_mutex_unlock(&(semaforo_memoria));

		log_info(logger, "MENSAJE ALMACENADO EN PARTICION:%d -- BIT DE INICIO:%d", entra, particion_a_llenar_con_msg->bit_inicio);; // LOG 6 EN HEXA

		pthread_mutex_unlock(&(semaforo_struct_s));
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
	printf("Buscar en bs\n");
	t_struct_secundaria* particion;
	int encontro_particion = 1;
	int tamanio_lista = list_size(lista_de_particiones);
	if (string_equals_ignore_case(algoritmo_part_libre, "FF")) {
		pthread_mutex_lock(&(semaforo_struct_s));
		for (int i = 0; i < tamanio_lista; i++) { // list_iterate podemos usar por ahi (despues verlo)
			particion = list_get(lista_de_particiones, i);
			if (particion->tipo_mensaje == 6) {
				while (particion->tamanio >= mayor_entre_Min_y_tam(tamanio_a_ocupar) && encontro_particion != 0) { // divide las particiones (64 -- 32 32 -- 16 16 32 ..)
					if (particion->tamanio / 2 >= mayor_entre_Min_y_tam(tamanio_a_ocupar)) {
						t_struct_secundaria* particion_nueva = duplicar_estructura(particion);
						particion->tamanio = particion->tamanio / 2;
						particion_nueva->tamanio = particion->tamanio; // ya va a estar divido el tamanio
						particion_nueva->bit_inicio = particion->bit_inicio + particion->tamanio;
						printf("Tamanio:%d\n",particion->tamanio);
						printf("Bit de inicio:%d\n",particion->bit_inicio);
						printf("Tamanio Part Nueva:%d\n",particion_nueva->tamanio);
						printf("Bit de inicio Part Nueva:%d\n",particion_nueva->bit_inicio);
						list_add_in_index(lista_de_particiones, i + 1, particion_nueva);
					} else {
						encontro_particion = 0;
						entra = i; // Encuentra la posicion que entra. Sigue recorriendo la lista por si no encuentra la particion adecuada y haya que eliminar y consolidar.
					}
				}
			}
		}
		pthread_mutex_unlock(&(semaforo_struct_s));
		if (entra == -1) {
			elegir_victima_para_eliminar_mediante_FIFO_o_LRU_bs(); // Como esta en BS entonces elimino y veo si puedo consolidar
		}
	} else if (string_equals_ignore_case(algoritmo_part_libre, "BF")) {
		int sobra;
		int sobra_menor = tamanio_memoria;
		for (int i = 0; i < list_size(lista_de_particiones); i++) {
			pthread_mutex_lock(&(semaforo_struct_s));
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
			pthread_mutex_unlock(&(semaforo_struct_s));
		}
		// termina de recorrer la lista, encuentra la particion mediante "BF" y luego hace bs para asignarle el tamanio justo a "entra"
		pthread_mutex_lock(&(semaforo_struct_s));
		particion = list_get(lista_de_particiones, entra);
		while (particion->tamanio >= mayor_entre_Min_y_tam(tamanio_a_ocupar) && encontro_particion != 0) {
			if (particion->tamanio / 2 >= mayor_entre_Min_y_tam(tamanio_a_ocupar)) {
				t_struct_secundaria* particion_nueva = duplicar_estructura(particion);
				particion->tamanio = particion->tamanio / 2;
				particion_nueva->tamanio = particion->tamanio;
				particion_nueva->bit_inicio = particion->bit_inicio + particion->tamanio;

				list_add_in_index(lista_de_particiones, entra + 1, particion_nueva);

			} else {
				encontro_particion = 0;
			}

		}
		pthread_mutex_unlock(&(semaforo_struct_s));
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
	printf("Son buddies\n");
	printf("tamanio A= %d\n",particion_A->tamanio);
	printf("tamanio B= %d\n",particion_B->tamanio);
	printf("bit de inicio A= %d\n",particion_A->bit_inicio);
	printf("bit de inicio B= %d\n",particion_B->bit_inicio);
	int operacion_xorA_B = particion_B->bit_inicio ^ particion_A->tamanio;
	printf("XorAB=%d\n",operacion_xorA_B);
	bool xorA_B = particion_A->bit_inicio == operacion_xorA_B;
	int operacion_xorB_A = particion_A->bit_inicio ^ particion_B->tamanio;
	printf("XorBA=%d\n",operacion_xorB_A);
	bool xorB_A = particion_B->bit_inicio == operacion_xorB_A;
	return particion_A->tamanio == particion_B->tamanio && xorA_B && xorB_A;
}

bool es_potencia_de_dos(int numero){
	printf("Es potencia\n");
	return ((numero!=0) && ((numero & (numero-1)) == 0)); // & = OPERACION LOGICA AND
}

void consolidar_particiones_en_bs(int posicion_a_liberar) { // Consolido cada vez que se libera una particion y sigo consolidando hasta que no pueda mas
	printf("Consolidar particiones\n");
	pthread_mutex_lock(&(semaforo_struct_s));
	t_struct_secundaria* particion_a_consolidar = list_get(lista_de_particiones, posicion_a_liberar);
	int tam_lista = list_size(lista_de_particiones);
	for (int i = 0; i < tam_lista; i++) {
		printf("i Arranque:%d\n",i);
		t_struct_secundaria* particion_a_comparar_si_es_buddy = list_get(lista_de_particiones, i);
		if(son_buddies(particion_a_consolidar, particion_a_comparar_si_es_buddy) && particion_a_comparar_si_es_buddy->tipo_mensaje == 6) {
			printf("particion_a_consolidar->bit_inicio: %d\n",particion_a_consolidar->bit_inicio);
			printf("particion_a_comparar_si_es_buddy->bit_inicio: %d\n",particion_a_comparar_si_es_buddy->bit_inicio);
			if(particion_a_consolidar->bit_inicio > particion_a_comparar_si_es_buddy->bit_inicio){
				particion_a_consolidar->bit_inicio = particion_a_comparar_si_es_buddy->bit_inicio;
			}
			particion_a_consolidar->tamanio = particion_a_comparar_si_es_buddy->tamanio + particion_a_consolidar->tamanio;
			log_info(logger,
					"ASOCIACION DE PARTICION:%d -- BIT DE INCICIO:%d y PARTICION:%d -- BIT DE INCICIO:%d ",
					posicion_a_liberar, particion_a_comparar_si_es_buddy->bit_inicio, i,
					particion_a_consolidar->bit_inicio); // LOG 8
			printf("particion que queda-> tamanio: %d - bit inicio: %d\n",particion_a_consolidar->tamanio,particion_a_consolidar->bit_inicio);
			list_remove_and_destroy_element(lista_de_particiones, i, free);
			tam_lista -=1;
			i -= 2;
			if (i<0){
				i = -1;
			}
			printf("i Final=%d\n",i);
		}
	}
	pthread_mutex_unlock(&(semaforo_struct_s));
}

void buscar_particion_en_particiones_dinamicas(){
	t_struct_secundaria* nueva_est;

	if(string_equals_ignore_case(algoritmo_part_libre,"FF")){
		for (int i = 0; i < list_size(lista_de_particiones); i++ ){
			pthread_mutex_lock(&(semaforo_struct_s));
			nueva_est = list_get(lista_de_particiones, i);
			if (nueva_est->tipo_mensaje == 6){
				if(nueva_est->tamanio >= mayor_entre_Min_y_tam(tamanio_a_ocupar) ){
					entra = i;
				}
			}
			pthread_mutex_unlock(&(semaforo_struct_s));
		}

		if (entra == -1){
			if (flag == frecuencia_compactacion){
				compactar();
			}else if (frecuencia_compactacion == -1 && flag == -1){
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
			pthread_mutex_lock(&(semaforo_struct_s));
			nueva_est = list_get(lista_de_particiones,i);
			if (nueva_est->tipo_mensaje == 6){
				if(nueva_est->tamanio >= mayor_entre_Min_y_tam(tamanio_a_ocupar) ){
					sobra = (nueva_est->tamanio - mayor_entre_Min_y_tam(tamanio_a_ocupar));
					if (sobra < sobra_menor){
						sobra_menor = sobra;
						entra = i;
					}
				}
			}
			pthread_mutex_unlock(&(semaforo_struct_s));
		}
		if (entra == -1){
			if (flag == frecuencia_compactacion){
				compactar();
			}else if (frecuencia_compactacion == -1 && flag == -1){
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
		pthread_mutex_lock(&(semaforo_struct_s));
		estructura1 = list_get(lista_de_particiones,i);
		if (estructura1->tipo_mensaje == 6 && i+1 < tamanio_lista_actual){
			estructura2 = list_get(lista_de_particiones,(i+1));
			if(estructura2->tipo_mensaje == 6){
				log_info(logger, "COMPACTACION DE PARTICION:%d -- BIT DE INCICIO:0x%x y PARTICION:%d -- BIT DE INCICIO:0x%x ",
								i, estructura1->bit_inicio, (i+1),
								estructura2->bit_inicio); // LOG 8

				estructura1->tamanio += estructura2->tamanio;

				list_remove_and_destroy_element(lista_de_particiones,(i+1),free);
				tamanio_lista_actual -= 1;
				i -= 1;
			} else {

				mover_memoria(i);

				actualizar_bit_inicio(i);

				t_struct_secundaria* nueva = duplicar_estructura(estructura1);
				nueva->bit_inicio = tamanio_memoria - estructura1->tamanio;
				nueva->id_mensaje = 0;
				nueva->tamanio = estructura1->tamanio;
				nueva->tipo_mensaje = 6;

				list_add(lista_de_particiones, nueva);
				list_remove_and_destroy_element(lista_de_particiones,i,free);
				i -= 1;
			}
		}
		pthread_mutex_unlock(&(semaforo_struct_s));
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
	pthread_mutex_lock(&(semaforo_struct_s));
	t_struct_secundaria* particion_a_sacar = list_get(lista_de_particiones,a_sacar);

	sacar_de_cola(particion_a_sacar->id_mensaje,particion_a_sacar->tipo_mensaje );

	log_info(logger,"ELIMINO PARTICION:%d -- BIT DE INCICIO:%d", a_sacar, particion_a_sacar->bit_inicio); // LOG 7
	particion_a_sacar->id_mensaje = -1;
	particion_a_sacar->tipo_mensaje= 6;
	particion_a_sacar->tamanio = mayor_entre_Min_y_tam(particion_a_sacar->tamanio);

	pthread_mutex_unlock(&(semaforo_struct_s));

	bool esVacio(void* una_particion){
		t_struct_secundaria* particion_a_comparar =  (t_struct_secundaria*) una_particion;
		return particion_a_comparar->tipo_mensaje == 6;
	}
	pthread_mutex_lock(&(semaforo_struct_s));
	if (list_all_satisfy(lista_de_particiones, esVacio)){
		flag = -1;
	}
	pthread_mutex_unlock(&(semaforo_struct_s));

	buscar_particion_en_particiones_dinamicas();
}

void elegir_victima_para_eliminar_mediante_FIFO_o_LRU_bs() {
	printf("ELIMINAR VICTIMA\n");
	int a_sacar = 0;

	if (string_equals_ignore_case(algoritmo_remplazo, "FIFO")) {
		a_sacar = algoritmo_FIFO();
	} else if (string_equals_ignore_case(algoritmo_remplazo, "LRU")) {
		a_sacar = algoritmo_LRU();
	} else {
		printf("Error en broker.config ALGORITMO_REEMPLAZO no valido");
	}

	pthread_mutex_lock(&(semaforo_struct_s));
	t_struct_secundaria* particion_a_sacar = list_get(lista_de_particiones, a_sacar);

	sacar_de_cola(particion_a_sacar->id_mensaje, particion_a_sacar->tipo_mensaje);

	log_info(logger,"ELIMINO PARTICION:%d -- BIT DE INCICIO:%d", a_sacar, particion_a_sacar->bit_inicio); // LOG 7
	particion_a_sacar->id_mensaje = 0;
	particion_a_sacar->tipo_mensaje = 6;


	while (!es_potencia_de_dos(particion_a_sacar->tamanio)) { // Le sumo 1 hasta ver si es potencia de 2 ya que la estructura >= al size que tenia antes (ejemplo: tamanio = 6 entonces 7 ..8 para ahi)
		particion_a_sacar->tamanio += 1;
	}
	particion_a_sacar->tamanio = mayor_entre_Min_y_tam(particion_a_sacar->tamanio);
	printf("Tamanio de particion eliminada:%d\n",particion_a_sacar->tamanio);
	particion_a_sacar->auxiliar = 0;
	pthread_mutex_unlock(&(semaforo_struct_s));

	consolidar_particiones_en_bs(a_sacar);
	buscar_particion_en_bs();
}

int algoritmo_FIFO(){
	printf("FIFO\n");
	t_struct_secundaria* particion_a_sacar;
	int orden = 0;
	int orden_menor = 0;
	int a_sacar = -1; // Creo q esta bien esto
	int a = 0;
	for(int i = 0; i< list_size(lista_de_particiones); i++ ){
		pthread_mutex_lock(&(semaforo_struct_s));
		particion_a_sacar = list_get(lista_de_particiones,i);
		if(i == a && particion_a_sacar->tipo_mensaje != 6 ){
			orden_menor = particion_a_sacar->auxiliar;
			printf("Orden MENOR =%d\n",orden_menor);
			a_sacar = i;
		} else {
			a += 1;
		}
		orden = particion_a_sacar->auxiliar;
		printf("Orden =%d\n",orden);
		if (orden_menor > orden && particion_a_sacar->tipo_mensaje != 6){
			orden_menor = orden;
			printf("Resultado orden menor =%d\n",orden_menor);
			a_sacar = i;
			printf("A SACAR EN IF=%d\n",a_sacar);
		}
		pthread_mutex_unlock(&(semaforo_struct_s));
	}
	printf("A SACAR REAL =%d\n",a_sacar);
	return a_sacar;
}

int algoritmo_LRU(){ //arreglar cuando haga lo de la hora

	printf("LRU\n");
	t_struct_secundaria* particion_a_sacar;
	int contador = 0;
	int flag_2 = 0;
	int a_sacar = 0 ;
	do {
		if (contador == list_size(lista_de_particiones)){
			contador = 0;
		}
		pthread_mutex_lock(&(semaforo_struct_s));
		particion_a_sacar = list_get(lista_de_particiones,contador);
		if (particion_a_sacar->auxiliar == 0 ){
			a_sacar = contador;
			flag_2 = 1;
		} else {
			particion_a_sacar->auxiliar = 0;

		}
		pthread_mutex_unlock(&(semaforo_struct_s));
		contador ++;
	} while(flag_2 == 0);

	return a_sacar;
}

void actualizar_bit_inicio(int a_sacar){ // actualiza el bit de inicio y lo elimina de la lista de particiones a la particion a sacar // Creo que asi esta perfecto
	printf("Actualizar bit\n");
	t_struct_secundaria* particion_actual; // en donde estoy parado ahora
	t_struct_secundaria* particion_a_sacar = list_get(lista_de_particiones,a_sacar);

	for (int f = a_sacar + 1; f < list_size(lista_de_particiones); f++) { // Siempre haces que el bit inicio siguiente se mueva para atras
		particion_actual = list_get(lista_de_particiones, f);
		particion_actual->bit_inicio = particion_actual->bit_inicio - mayor_entre_Min_y_tam(particion_a_sacar->tamanio);// bit de inicio del siguinte le resta el tamanio del a sacar
	}
}

void mover_memoria(int a_sacar) {
	printf("Mover memoria\n");
	t_struct_secundaria* particion_a_mover_memoria;
	particion_a_mover_memoria = list_get(lista_de_particiones, a_sacar);
	int tamanio_a_mover = tamanio_memoria - (particion_a_mover_memoria->bit_inicio + mayor_entre_Min_y_tam(particion_a_mover_memoria->tamanio));
	void* comienzo_a_sacar = memoria + particion_a_mover_memoria->bit_inicio;
	void* de_donde = memoria + particion_a_mover_memoria->bit_inicio + mayor_entre_Min_y_tam(particion_a_mover_memoria->tamanio);
	// de donde voy a sacar la memoria a mover

	pthread_mutex_lock(&(semaforo_memoria));
	memmove(comienzo_a_sacar, de_donde, tamanio_a_mover);
	pthread_mutex_unlock(&(semaforo_memoria));

}

void* de_id_mensaje_a_mensaje(uint32_t id_mensaje){
	printf("id mensaje a mensaje\n");
	t_struct_secundaria* particion_de_donde_voy_a_sacar_el_tipo_de_cola = encontrar_particion_en_base_a_un_id_mensaje(id_mensaje);
	void* mensaje = malloc(particion_de_donde_voy_a_sacar_el_tipo_de_cola->tamanio);
	pthread_mutex_lock(&(semaforo_memoria));
	memcpy(mensaje, memoria + particion_de_donde_voy_a_sacar_el_tipo_de_cola->bit_inicio, particion_de_donde_voy_a_sacar_el_tipo_de_cola->tamanio);
	pthread_mutex_unlock(&(semaforo_memoria));
	free(particion_de_donde_voy_a_sacar_el_tipo_de_cola);
	return mensaje;
}

uint32_t de_id_mensaje_a_cola(uint32_t id_mensaje){
	printf("id mensaje a cola\n");
	t_struct_secundaria* particion_de_donde_voy_a_sacar_el_tipo_de_cola = encontrar_particion_en_base_a_un_id_mensaje(id_mensaje);
	uint32_t cola = particion_de_donde_voy_a_sacar_el_tipo_de_cola->tipo_mensaje;
	free(particion_de_donde_voy_a_sacar_el_tipo_de_cola);
	return cola;
}


uint32_t de_id_mensaje_a_size(uint32_t id_mensaje){ //revisar
	printf("id mensaje a size\n");
	t_struct_secundaria* particion_de_donde_voy_a_sacar_el_tipo_de_cola = encontrar_particion_en_base_a_un_id_mensaje(id_mensaje);
	uint32_t size = particion_de_donde_voy_a_sacar_el_tipo_de_cola->tamanio;
	free(particion_de_donde_voy_a_sacar_el_tipo_de_cola);
	return size;
}

t_struct_secundaria* encontrar_particion_en_base_a_un_id_mensaje(uint32_t id_mensaje){
	printf("Encontrar particion\n");
	t_struct_secundaria* particion_de_donde_voy_a_sacar_el_tipo_de_cola;
	bool mismo_id(void* una_particion){
		t_struct_secundaria* particion_a_comparar =  (t_struct_secundaria*) una_particion;
		return particion_a_comparar->id_mensaje == id_mensaje;
	}
	printf("MISMO ID LO HACE\n");
	pthread_mutex_lock(&(semaforo_struct_s));
	printf("LOCK\n");
	particion_de_donde_voy_a_sacar_el_tipo_de_cola = list_find(lista_de_particiones, mismo_id);
	t_struct_secundaria* particion = duplicar_estructura(particion_de_donde_voy_a_sacar_el_tipo_de_cola);
	pthread_mutex_unlock(&(semaforo_struct_s));
	printf("UNLOCK\n");
	return particion;
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
		fprintf(dump_file,"0x%x - 0x%x.\t ",particion_a_mostrar->bit_inicio, particion_a_mostrar->bit_inicio + mayor_entre_Min_y_tam(particion_a_mostrar->tamanio));
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

int mayor_entre_Min_y_tam(int tamanio){
	printf("mayor entre min y tam\n");
	if( tamanio_minimo > tamanio){
		return tamanio_minimo;
	}else{
		return tamanio;
	}

}
