CC = gcc -Wall

MAIN = ejecutable

SOURCES = ej4.c rutinas_clock.c

LIBS = rutinas_clock.h

$(MAIN): $(SOURCES) $(LIBS)
	$(CC) -o $(MAIN) $(SOURCES) -lm -O0 -fopenmp
