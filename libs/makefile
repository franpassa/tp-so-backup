all: libcuarenteam.so

libcuarenteam.so: conexiones.o
	gcc -shared -g -o libcuarenteam.so conexiones.o

conexiones.o: conexiones.c conexiones.h
	gcc -c -Wall -g -Werror -fpic conexiones.c

install: libcuarenteam.so
	cp -u libcuarenteam.so /usr/lib
	cp --parents -u conexiones.h /usr/include	

clean: # Finalmente creamos una tarea para eliminar todos los .o y el binario
	-rm -f *.o
	-rm -f libcuarenteam.so