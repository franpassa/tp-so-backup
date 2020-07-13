all: libcuarenteam.so

libcuarenteam.so:
	gcc  -shared -g -O0 -o libcuarenteam.so -fPIC conexiones.c

install: libcuarenteam.so
	cp -u libcuarenteam.so /usr/lib
	cp --parents -u conexiones.h /usr/include

clean: # Finalmente creamos una tarea para eliminar todos los .o y el binario
	-rm -f *.o
	-rm -f cuarenteam.so