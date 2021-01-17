#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <omp.h>
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
#define k 1 // Numero de hilos

unsigned long int N; // Tamano del vector de cuaterniones: N = 10 ^ q


// Definicion de funciones
void inicializacion(cuaternions *a, cuaternions *b);


//
int main(int argc, char *argv[])
{
	FILE *archivo = fopen(argv[1], "a"); // archivo donde guardaremos los datos
	
	// Variables
	cuaternions *a, *b; // Vectores de cuaterniones iniciales, con los que se opera
	cuaternions c; // Cuaternion auxiliar, almacena la primera multiplicacion
	cuaternions auxdp; // Cuaternion auxiliar con las sumas parciales para cada hilo
	cuaternions dp; // Cuaternion final, resultado de las operaciones
	double ck; // Contador de ciclos
	int i;
	
	// Inicializaciones
	N = pow(10, q); //N = 10 ^ q
	dp.c1 = 0, dp.c2 = 0, dp.c3 = 0, dp.c4 = 0; // Inicializacion del vector resultado, sobre esto se acumulara la suma final

	// Asignacion de memoria
	a = malloc(N * sizeof(cuaternions));
	b = malloc(N * sizeof(cuaternions));

	// Inicializacion de los vectores
	inicializacion(a, b); // Se dan valores a los vectores de cuaterniones

	// Comienzo de la medicion de ciclos
	start_counter();

	// El siguiente fragmento de codigo se va a paralelizar, estableciendo 'k' hilos y una serie de variables privadas para cada uno
	#pragma omp parallel private(i, auxdp, c) num_threads(k)
	{
		// Se inicializan las variables privadas necesarias
		auxdp.c1 = 0, auxdp.c2 = 0, auxdp.c3 = 0, auxdp.c4 = 0;

		// En el siguiente bucle, se reparten las iteraciones entre los diferentes hilos
		#pragma omp for
		for (i = 0; i < N; i++)
		{
			// Primero se realiza el producto de dos cuaterniones de los vectores
			c.c1 = a[i].c1 * b[i].c1 - a[i].c2 * b[i].c2 - a[i].c3 * b[i].c3 - a[i].c4 * b[i].c4;
			c.c2 = a[i].c1 * b[i].c2 + a[i].c2 * b[i].c1 + a[i].c3 * b[i].c4 - a[i].c4 * b[i].c3;
			c.c3 = a[i].c1 * b[i].c3 - a[i].c2 * b[i].c4 + a[i].c3 * b[i].c1 + a[i].c4 * b[i].c2;
			c.c4 = a[i].c1 * b[i].c4 + a[i].c2 * b[i].c3 - a[i].c3 * b[i].c2 + a[i].c4 * b[i].c1;

			// Despues se realiza la adicion de los cuaterniones, junto con el producto del cuaternion c consigo mismo, como una suma parcial
			auxdp.c1 += c.c1 * c.c1 - c.c2 * c.c2 - c.c3 * c.c3 - c.c4 * c.c4;
			auxdp.c2 += c.c1 * c.c2 + c.c2 * c.c1 + c.c3 * c.c4 - c.c4 * c.c3;
			auxdp.c3 += c.c1 * c.c3 - c.c2 * c.c4 + c.c3 * c.c1 + c.c4 * c.c2;
			auxdp.c4 += c.c1 * c.c4 + c.c2 * c.c3 - c.c3 * c.c2 + c.c4 * c.c1;
		}

		// Al final, un solo hilo de forma simultanea realiza la adicion de la suma parcial a la total, evitando condiciones de carrerra
		#pragma omp atomic
			dp.c1 += auxdp.c1;
		#pragma omp atomic
			dp.c2 += auxdp.c2;
		#pragma omp atomic
			dp.c3 += auxdp.c3;
		#pragma omp atomic
			dp.c4 += auxdp.c4;
	}

	// Fin de la medicion de ciclos
	ck = get_counter();

	// Impresion de resulados
	printf("[q: %u / N: %ld]  =>  ciclos/cuaternion = %1.10lf\n", q, N, ck/N); // Tam del vector y numero de ciclos por cuaternion
	printf("\tdp.1 = %lf\n\tdp.2 = %lf\n\tdp.3 = %lf\n\tdp.4 = %lf\n", dp.c1, dp.c2, dp.c3, dp.c4); // Valores de los componentes
	
	fprintf(archivo, "%lf\n", ck / N); //medimos los tiempos por cada cuaternión

	// Liberacion de memoria
	free(a);
	free(b);
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
