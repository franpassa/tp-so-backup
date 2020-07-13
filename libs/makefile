all: libcuarenteam.so

libcuarenteam.so: conexiones.o
	gcc -shared -g -O0 conexiones.o

conexiones.o: conexiones.c conexiones.h
	gcc -c -Wall -g -O0 -fPIC conexiones.c

install: libcuarenteam.so
	cp -u libcuarenteam.so /usr/lib
	cp --parents -u conexiones.h /usr/include

clean: # Finalmente creamos una tarea para eliminar todos los .o y el binario
	-rm -f *.o
	-rm -f cuarenteam.so