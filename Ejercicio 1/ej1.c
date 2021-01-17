#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include "rutinas_clock.h"


// Estructura del cuaternion
typedef struct
{
	double c1, c2, c3, c4; // compuesta por 4 doubles
} cuaternions;


// Datos
#define CUAT 4 // Numero de componentes del cuaternion
#define MAX_RAND 1000 // Maximo rango de los aleatorios (rand % MAX_RAND)
#define q 2 // Decide el valor de N, probar con valores: 2, 4, 6, 7

unsigned long int N; // Tamano del vector de cuaterniones: N = 10 ^ q


// Definicion de funciones
void inicializacion(cuaternions *a, cuaternions *b);
cuaternions *multiplicacion(cuaternions *a, cuaternions *b);
void adicion(cuaternions *dp, cuaternions *a);


//
int main(int argc, char *argv[])
{
	FILE *archivo = fopen(argv[1], "a"); // archivo donde guardaremos los datos
    	
	// Variables
	cuaternions *a, *b; // Vectores de cuaterniones iniciales, con los que se opera
	cuaternions *c; // Vector de cuaterniones auxiliar, almacena la primera multiplicacion
	cuaternions dp; // Cuaternion final, resultado de las operaciones
	double ck; // Contador de ciclos
	
	// Inicializaciones
	N = pow(10, (int) q); //N = 10 ^ q
	dp.c1 = 0; // Inicializacion del vector resultado
	dp.c2 = 0; // sobre esto se acumulara la suma final
	dp.c3 = 0;
	dp.c4 = 0;

	// Asignacion de memoria
	a = malloc(N * sizeof(cuaternions));
	b = malloc(N * sizeof(cuaternions));
	c = malloc(N * sizeof(cuaternions));

	// Inicializacion de los vectores
	inicializacion(a, b); // Se dan valores a los vectores de cuaterniones

	// Comienzo de la medicion de ciclos
	start_counter();

	/* OPERACIONES */
	// Multiplicacion de los vectores de cuaterniones: c = a*b
	c = multiplicacion(a, b);
	// Multiplicacion sobre si misma del productor anterior y suma sobre los cuaterniones: dp = dp+c*c
	adicion(&dp, multiplicacion(c, c));

	// Fin de la medicion de ciclos
	ck = get_counter();

	// Impresion de resulados
	printf("[q: %u / N: %ld]  =>  ciclos/cuaternion = %1.10lf\n", q, N, ck/N); // Tam del vector y numero de ciclos por cuaternion
	printf("\tdp.1 = %lf\n\tdp.2 = %lf\n\tdp.3 = %lf\n\tdp.4 = %lf\n", dp.c1, dp.c2, dp.c3, dp.c4); // Valores de los componentes
	
	fprintf(archivo, "%lf\n", ck / N); //medimos los tiempos por cada cuaternión

	// Liberacion de memoria
	free(a);
	free(b);
	free(c);
}


/* Inicializacion de los (4) componentes de cada cuaternion de los dos vectores
 *  - Argumentos: vector de cuaterniones, vector de cuaterniones
 *  - Resultado: ... <= los cambios se realizan directamente sobre los parametros
 */
void inicializacion(cuaternions *a, cuaternions *b)
{
	// Se asigna una semilla para la generacion de aleatorios (la misma que en los otros programas)
	srand(1);

	for (unsigned long int i = 0; i < N; i++)
	{
		// Asignacion de un valor aleatorio entre 0 y MAX_RAND para cada componente
		a[i].c1 = rand() % MAX_RAND;
		a[i].c2 = rand() % MAX_RAND;
		a[i].c3 = rand() % MAX_RAND;
		a[i].c4 = rand() % MAX_RAND;
		b[i].c1 = rand() % MAX_RAND;
		b[i].c2 = rand() % MAX_RAND;
		b[i].c3 = rand() % MAX_RAND;
		b[i].c4 = rand() % MAX_RAND;
	}
}


/* Multiplicacion de dos vectores de cuaterniones: producto cuaternion a cuaternion
 *  - Argumentos: vector de cuaterniones, vector de cuaterniones
 *  - Resultado: vector de cuaterniones
 */
cuaternions *multiplicacion(cuaternions *a, cuaternions *b)
{
	// El resultado sera otro vector de cuaterniones
	cuaternions *resultado = malloc(N * sizeof(cuaternions));

	for (unsigned long int i = 0; i < N; i++)
	{
		// A cada iteracion se realiza el producto de dos cuaterniones, de la misma posicion, dando como resultado otro cuaternion
		// El producto de dos cuaterniones es de la siguiente manera:
		resultado[i].c1 = a[i].c1 * b[i].c1 - a[i].c2 * b[i].c2 - a[i].c3 * b[i].c3 - a[i].c4 * b[i].c4;
		resultado[i].c2 = a[i].c1 * b[i].c2 + a[i].c2 * b[i].c1 + a[i].c3 * b[i].c4 - a[i].c4 * b[i].c3;
		resultado[i].c3 = a[i].c1 * b[i].c3 - a[i].c2 * b[i].c4 + a[i].c3 * b[i].c1 + a[i].c4 * b[i].c2;
		resultado[i].c4 = a[i].c1 * b[i].c4 + a[i].c2 * b[i].c3 - a[i].c3 * b[i].c2 + a[i].c4 * b[i].c1;
	}

	return resultado;
}


/* Suma de un vector de cuaterniones: suma acumulada de cada componente
 *  - Argumentos: cuaternion resultado, vector de cuaterniones
 *  - Resultado: ... <= los cambios se realizan directamente sobre los parametros
 */
void adicion(cuaternions *dp, cuaternions *a)
{
	for (unsigned long int i = 0; i < N; i++)
	{
		// Para la suma, se acumula el valor de la componente de cada cuaternion
		dp->c1 += a[i].c1;
		dp->c2 += a[i].c2;
		dp->c3 += a[i].c3;
		dp->c4 += a[i].c4;
	}
}
