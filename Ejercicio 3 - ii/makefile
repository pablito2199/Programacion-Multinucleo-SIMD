CC = gcc -Wall

MAIN = ejecutable

SOURCES = ej3b.c rutinas_clock.c

LIBS = rutinas_clock.h

$(MAIN): $(SOURCES) $(LIBS)
	$(CC) -o $(MAIN) $(SOURCES) -lm -m32 -mavx2 -mfma -O0
