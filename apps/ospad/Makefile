CC=$(ARM)gcc
AR=$(ARM)ar

all:
	$(CC) -o ospad ospad.c -Wall -DDEBUG -g

lib:
	$(CC) -o ospad.o -c ospad.c -Wall
	$(AR) rcs libospad.a ospad.o

lib-shared:
	$(CC) -o ospad.o -c ospad.c -fPIC -Wall
	$(CC) -shared -Wl,-soname,libospad.so.1 -o libospad.so.1.0.1  ospad.o

clean:
	rm *.o libospad.a libospad.so.1.0.1 ospad;