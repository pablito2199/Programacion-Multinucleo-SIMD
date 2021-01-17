CC = gcc -Wall

MAIN = ejecutable

SOURCES = ej3a.c rutinas_clock.c

LIBS = rutinas_clock.h

$(MAIN): $(SOURCES) $(LIBS)
	$(CC) -o $(MAIN) $(SOURCES) -lm -m32 -mavx2 -O0
