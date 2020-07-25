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
	cont_orden = 0;

	particion_inicial->tipo_mensaje = 6;
	particion_inicial->tamanio = tamanio_memoria;
	particion_inicial->id_mensaje = 0;
	particion_inicial->bit_inicio = 0;
	particion_inicial->auxiliar = -1;
	particion_inicial->id_correlativo = 0;
	list_add(lista_de_particiones, particion_inicial);

}

void almacenar(void* mensaje, uint32_t id_cola, uint32_t id_mensaje, uint32_t size,uint32_t id_correlativo){

	entra = -1;
	tamanio_a_ocupar = size;

	if(string_equals_ignore_case(algoritmo_memoria,"PARTICIONES")){

		buscar_particion_en_particiones_dinamicas();

		pthread_mutex_lock(&(semaforo_struct_s));
		t_struct_secundaria* estructura_memoria = (t_struct_secundaria*) list_get(lista_de_particiones, entra); // en lista

		if (estructura_memoria->tamanio > mayor_entre_Min_y_tam(size)){
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
			estructura_memoria->auxiliar = f_cont_lru();
		} else {
			printf("Error en broker.config ALGORITMO_REEMPLAZO no valido");
		}

		estructura_memoria->id_mensaje = id_mensaje;
		estructura_memoria->tamanio = size;
		estructura_memoria->tipo_mensaje = id_cola;
		estructura_memoria->id_correlativo = id_correlativo;
		estructura_memoria->a_quienes_fue_enviado = list_create();
		estructura_memoria->quienes_lo_recibieron = list_create();
        pthread_mutex_lock(&(semaforo_memoria));
        memcpy(memoria + estructura_memoria->bit_inicio, mensaje, size);
        pthread_mutex_unlock(&(semaforo_memoria));

        log_info(logger, "MENSAJE ALMACENADO EN PARTICION:%d -- BIT DE INICIO:%d", entra, estructura_memoria->bit_inicio); // LOG 6

        pthread_mutex_unlock(&(semaforo_struct_s));

	} else if(string_equals_ignore_case(algoritmo_memoria,"BS")) {

		buscar_particion_en_bs(); // la lista de particiones va a tener mas de 1 elemento al hacer esto
		pthread_mutex_lock(&(semaforo_struct_s));
		t_struct_secundaria* particion_a_llenar_con_msg = (t_struct_secundaria*) list_get(lista_de_particiones,entra);

		particion_a_llenar_con_msg->tamanio = size;
		particion_a_llenar_con_msg->id_mensaje = id_mensaje;
		particion_a_llenar_con_msg->tipo_mensaje = id_cola;
		particion_a_llenar_con_msg->id_correlativo = id_correlativo;
		particion_a_llenar_con_msg->a_quienes_fue_enviado = list_create();
		particion_a_llenar_con_msg->quienes_lo_recibieron = list_create();

		if(string_equals_ignore_case(algoritmo_remplazo,"FIFO")) {
			cont_orden ++;
			particion_a_llenar_con_msg->auxiliar = cont_orden;
		} else if(string_equals_ignore_case(algoritmo_remplazo,"LRU")) {
			particion_a_llenar_con_msg->auxiliar = f_cont_lru();
		} else {
			printf("Error en broker.config ALGORITMO_REEMPLAZO no valido");
		}


		pthread_mutex_lock(&(semaforo_memoria));
		memcpy(memoria + particion_a_llenar_con_msg->bit_inicio, mensaje, size);
		pthread_mutex_unlock(&(semaforo_memoria));

		log_info(logger, "MENSAJE ALMACENADO EN PARTICION:%d -- BIT DE INICIO:%d", entra, particion_a_llenar_con_msg->bit_inicio);; // LOG 6

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
	duplicado->a_quienes_fue_enviado = estructura->a_quienes_fue_enviado;
	duplicado->quienes_lo_recibieron = estructura->quienes_lo_recibieron;

	return duplicado;
}

void buscar_particion_en_bs() {
	t_struct_secundaria* particion;
	int encontro_particion = 1;
	int tamanio_lista = list_size(lista_de_particiones);
	if (string_equals_ignore_case(algoritmo_part_libre, "FF")) {
		pthread_mutex_lock(&(semaforo_struct_s));
		for (int i = 0; i < tamanio_lista; i++) {
			particion = list_get(lista_de_particiones, i);
			if (particion->tipo_mensaje == 6) {
				while (particion->tamanio >= mayor_entre_Min_y_tam(tamanio_a_ocupar) && encontro_particion != 0) { // divide las particiones (64 -- 32 32 -- 16 16 32 ..)
					if (particion->tamanio / 2 >= mayor_entre_Min_y_tam(tamanio_a_ocupar)) {
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
	int operacion_xorA_B = particion_B->bit_inicio ^ particion_A->tamanio;
	bool xorA_B = particion_A->bit_inicio == operacion_xorA_B;
	int operacion_xorB_A = particion_A->bit_inicio ^ particion_B->tamanio;
	bool xorB_A = particion_B->bit_inicio == operacion_xorB_A;
	return particion_A->tamanio == particion_B->tamanio && xorA_B && xorB_A;
}

//bool es_potencia_de_dos(int numero){
//	return ((numero!=0) && ((numero & (numero-1)) == 0)); // & = OPERACION LOGICA AND
//}

bool es_potencia_de_dos(int numero){
	int a =1;
	while(potencia (2,a) <= numero){
		if (numero == potencia (2,a)){
			return 1;
		}else{
			a++;
		}
	}
	return 0;
}

int potencia (int base,int expo){
	int resultado = base;
	 for (int i=2 ;i<expo+1;i++){
		 resultado = resultado * base;
	 }
	 return resultado;
}




void consolidar_particiones_en_bs(int posicion_a_liberar) { // Consolido cada vez que se libera una particion y sigo consolidando hasta que no pueda mas
	int posicion = posicion_a_liberar;
	pthread_mutex_lock(&(semaforo_struct_s));
	t_struct_secundaria* particion_a_consolidar = list_get(lista_de_particiones, posicion_a_liberar);
	int tam_lista = list_size(lista_de_particiones);
	for (int i = 0; i < tam_lista; i++) {
		t_struct_secundaria* particion_a_comparar_si_es_buddy = list_get(lista_de_particiones, i);
		if(son_buddies(particion_a_consolidar, particion_a_comparar_si_es_buddy) && particion_a_comparar_si_es_buddy->tipo_mensaje == 6) {
			log_info(logger,"ASOCIACION DE PARTICION:%d -- BIT DE INICIO:%d y PARTICION:%d -- BIT DE INICIO:%d ",
							posicion, particion_a_consolidar->bit_inicio, i, particion_a_comparar_si_es_buddy->bit_inicio);
			if(particion_a_consolidar->bit_inicio > particion_a_comparar_si_es_buddy->bit_inicio){
				particion_a_consolidar->bit_inicio = particion_a_comparar_si_es_buddy->bit_inicio;
				posicion -= 1;
			}
			particion_a_consolidar->tamanio = particion_a_comparar_si_es_buddy->tamanio + particion_a_consolidar->tamanio;
			list_remove_and_destroy_element(lista_de_particiones, i, free);
			tam_lista -= 1;
			i -= 2;
			if (i < 0){
				i = -1;
			}
		}
	}
	pthread_mutex_unlock(&(semaforo_struct_s));
}

void buscar_particion_en_particiones_dinamicas(){
	t_struct_secundaria* nueva_est;
	if(string_equals_ignore_case(algoritmo_part_libre,"FF")){
		pthread_mutex_lock(&(semaforo_struct_s));
		for (int i = 0; i < list_size(lista_de_particiones); i++ ){
			nueva_est = list_get(lista_de_particiones, i);
			if (nueva_est->tipo_mensaje == 6){
				if(nueva_est->tamanio >= mayor_entre_Min_y_tam(tamanio_a_ocupar) ){
					entra = i;
				}
			}
		}
		pthread_mutex_unlock(&(semaforo_struct_s));
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
		pthread_mutex_lock(&(semaforo_struct_s));
		for (int i = 0; i< list_size(lista_de_particiones); i++ ){
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
		}
		pthread_mutex_unlock(&(semaforo_struct_s));

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
	int tamanio_lista_actual = list_size(lista_de_particiones);
	pthread_mutex_lock(&(semaforo_struct_s));
	for (int i = 0; i < tamanio_lista_actual; i++ ){
		estructura1 = list_get(lista_de_particiones,i);
		if (estructura1->tipo_mensaje == 6 && i+1 < tamanio_lista_actual){

			log_info(logger, "COMPACTACION DE PARTICION:%d -- BIT DE INCICIO:%d",i ,estructura1->bit_inicio); // LOG 8

			mover_memoria(i);

			actualizar_bit_inicio(i);

			int nuevo_bit_inicio = tamanio_memoria - estructura1->tamanio;

			if(nuevo_bit_inicio < 0){
				nuevo_bit_inicio = 0;
				printf("ARREGLAMOS ESTO\n");
			}
			estructura1->bit_inicio = nuevo_bit_inicio;

			list_add(lista_de_particiones, estructura1);
			list_remove(lista_de_particiones,i);
			tamanio_lista_actual-=1;
			i-=1;
		}
	}
	pthread_mutex_unlock(&(semaforo_struct_s));
	flag = 0;
	consolidar();
	buscar_particion_en_particiones_dinamicas();
}
void consolidar(){
	t_struct_secundaria* estructura1;
	t_struct_secundaria* estructura2;
	int tamanio_lista_actual = list_size(lista_de_particiones);
	pthread_mutex_lock(&(semaforo_struct_s));
	for (int i = 0; i < tamanio_lista_actual; i++ ){
		estructura1 = list_get(lista_de_particiones,i);
		if (estructura1->tipo_mensaje == 6 && i+1 < tamanio_lista_actual){
			estructura2 = list_get(lista_de_particiones,(i+1));
			if(estructura2->tipo_mensaje == 6){

				estructura1->tamanio += estructura2->tamanio;

				list_remove_and_destroy_element(lista_de_particiones,(i+1),free);
				tamanio_lista_actual -= 1;
				i -= 1;
			}
		}
	}
	pthread_mutex_unlock(&(semaforo_struct_s));
}

void elegir_victima_para_eliminar_mediante_FIFO_o_LRU_particiones() {
	int a_sacar = elegir_bit_aux_mas_viejo();
	if (a_sacar == -1) {
		consolidar();
		buscar_particion_en_particiones_dinamicas();
	} else {

		pthread_mutex_lock(&(semaforo_struct_s));
		t_struct_secundaria* particion_a_sacar = list_get(lista_de_particiones,
				a_sacar);

		log_info(logger, "ELIMINO PARTICION:%d -- BIT DE INCICIO:%d", a_sacar,
				particion_a_sacar->bit_inicio); // LOG 7
		particion_a_sacar->id_mensaje = 0;
		particion_a_sacar->tipo_mensaje = 6;
		particion_a_sacar->tamanio = mayor_entre_Min_y_tam(
				particion_a_sacar->tamanio);
		particion_a_sacar->id_correlativo = 0;
		particion_a_sacar->auxiliar = -1;
		list_destroy_and_destroy_elements(
				particion_a_sacar->a_quienes_fue_enviado, free);
		list_destroy_and_destroy_elements(
				particion_a_sacar->quienes_lo_recibieron, free);

		pthread_mutex_unlock(&(semaforo_struct_s));

		bool esVacio(void* una_particion) {
			t_struct_secundaria* particion_a_comparar =
					(t_struct_secundaria*) una_particion;
			return particion_a_comparar->tipo_mensaje == 6;
		}
		pthread_mutex_lock(&(semaforo_struct_s));
		if (list_all_satisfy(lista_de_particiones, esVacio)) {
			flag = -1;
		}
		pthread_mutex_unlock(&(semaforo_struct_s));
		consolidar();
		buscar_particion_en_particiones_dinamicas();
	}
}

void elegir_victima_para_eliminar_mediante_FIFO_o_LRU_bs() {

	int a_sacar = elegir_bit_aux_mas_viejo();
	if (a_sacar == -1) {
		printf("Entra bien al if\n");
		int k = list_size(lista_de_particiones);
		for(int l = 0 ; l < k; l++){
			consolidar_particiones_en_bs(l);
			k = list_size(lista_de_particiones);
		}
		printf("Salgo del For\n");
		buscar_particion_en_bs();
		printf("Termino de buscar en bs\n");
	} else {
		pthread_mutex_lock(&(semaforo_struct_s));
		t_struct_secundaria* particion_a_sacar = list_get(lista_de_particiones,
				a_sacar);
		log_info(logger, "ELIMINO PARTICION:%d -- BIT DE INICIO:%d", a_sacar,
				particion_a_sacar->bit_inicio); // LOG 7
		particion_a_sacar->id_mensaje = 0;
		particion_a_sacar->tipo_mensaje = 6;
		particion_a_sacar->id_correlativo = 0;
		list_destroy_and_destroy_elements(
				particion_a_sacar->a_quienes_fue_enviado, free);
		list_destroy_and_destroy_elements(
				particion_a_sacar->quienes_lo_recibieron, free);

		while (!es_potencia_de_dos(particion_a_sacar->tamanio)) { // Le sumo 1 hasta ver si es potencia de 2 ya que la estructura >= al size que tenia antes (ejemplo: tamanio = 6 entonces 7 ..8 para ahi)
			particion_a_sacar->tamanio += 1;
		}
		particion_a_sacar->tamanio = mayor_entre_Min_y_tam(
				particion_a_sacar->tamanio);
		particion_a_sacar->auxiliar = -1;
		pthread_mutex_unlock(&(semaforo_struct_s));

		consolidar_particiones_en_bs(a_sacar);
		buscar_particion_en_bs();
	}
}

int elegir_bit_aux_mas_viejo(){
	t_struct_secundaria* particion_a_sacar;
	int orden = 0;
	int orden_menor = 0;
	int a_sacar = -1;
	int a = 0;
	pthread_mutex_lock(&(semaforo_struct_s));

	bool esVacio(void* una_particion){
		t_struct_secundaria* particion_a_comparar =  (t_struct_secundaria*) una_particion;
		return particion_a_comparar->tipo_mensaje == 6;
	}

	if (list_all_satisfy(lista_de_particiones, esVacio)){
		printf("ENTRA MAL ESTAN TODAS VACIAS\n");
	}
	for(int i = 0; i< list_size(lista_de_particiones); i++ ){
		particion_a_sacar = list_get(lista_de_particiones,i);
		if(i == a && particion_a_sacar->tipo_mensaje != 6 ){
			orden_menor = particion_a_sacar->auxiliar;
			a_sacar = i;
		} else if(i == a){
			a += 1;
		}
		if(particion_a_sacar->tipo_mensaje != 6 ){
			orden = particion_a_sacar->auxiliar;
		}

		if (orden_menor > orden && particion_a_sacar->tipo_mensaje != 6){
			orden_menor = orden;;
			a_sacar = i;
		}
	}
	pthread_mutex_unlock(&(semaforo_struct_s));
	printf(" A SACAR = %d \n",a_sacar);
	return a_sacar;
}

void actualizar_bit_inicio(int a_sacar){ // actualiza el bit de inicio y lo elimina de la lista de particiones a la particion a sacar // Creo que asi esta perfecto
	t_struct_secundaria* particion_actual; // en donde estoy parado ahora
	t_struct_secundaria* particion_a_sacar = list_get(lista_de_particiones,a_sacar);

	for (int f = a_sacar + 1; f < list_size(lista_de_particiones); f++) { // Siempre haces que el bit inicio siguiente se mueva para atras
		particion_actual = list_get(lista_de_particiones, f);
		int nuevo_bit_inicio = particion_actual->bit_inicio - mayor_entre_Min_y_tam(particion_a_sacar->tamanio);

		if(nuevo_bit_inicio < 0){
			nuevo_bit_inicio = 0;
			printf("ARREGLAMOS ESTO\n");
		}
		particion_actual->bit_inicio = nuevo_bit_inicio; // bit de inicio del siguinte le resta el tamanio del a sacar
	}
}

void mover_memoria(int a_sacar) {
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

void* sacar_mensaje_de_memoria(uint32_t bit_inicio,uint32_t tamanio){
	void* mensaje = malloc(tamanio);
	pthread_mutex_lock(&(semaforo_memoria));
	memcpy(mensaje, memoria + bit_inicio, tamanio);
	pthread_mutex_unlock(&(semaforo_memoria));
	return mensaje;

}

void dump_de_cache(int sig) {
	if (sig == SIGUSR1) {
		log_info(logger, "EJECUCION DE DUMP DE CACHE\n"); // LOG 9
		t_struct_secundaria* particion_a_mostrar;
		FILE* dump_file = fopen("broker.dump", "w");
		if (dump_file == NULL) {
			printf("No se pudo abrir el archivo.\n");
			exit(1);
		} else {
			time_t fecha;
			struct tm *info;
			time(&fecha);
			info = localtime(&fecha);
			fprintf(dump_file, "Date:%s ", asctime(info)); // Esta impreso distinto, imprime el dia y el mes en string

			for (int i = 0; i < list_size(lista_de_particiones); i++) {
				particion_a_mostrar = list_get(lista_de_particiones, i);
				fprintf(dump_file, "Particion %d:", i);
				fprintf(dump_file, "0x%x - 0x%x.\t ",
						particion_a_mostrar->bit_inicio,
						particion_a_mostrar->bit_inicio
								+ mayor_entre_Min_y_tam(
										particion_a_mostrar->tamanio));
				if (particion_a_mostrar->tipo_mensaje == 6) {
					fprintf(dump_file, "[L]\t Size:%d b \n",
							particion_a_mostrar->tamanio);
				} else {
					fprintf(dump_file,
							"[X]\t Size:%d b \t %s:%d \t Cola:%d \t Id:%d \n",
							particion_a_mostrar->tamanio,
							algoritmo_remplazo,
							particion_a_mostrar->auxiliar,
							particion_a_mostrar->tipo_mensaje,
							particion_a_mostrar->id_mensaje);
				}
			}
		}

		fclose(dump_file);
	}
}

int mayor_entre_Min_y_tam(int tamanio){
	if( tamanio_minimo > tamanio){
		return tamanio_minimo;
	}else{
		return tamanio;
	}

}
int f_cont_lru(){
	pthread_mutex_lock(&(sem_lru));
	cont_lru ++;
	pthread_mutex_unlock(&(sem_lru));
	return cont_lru;
}

