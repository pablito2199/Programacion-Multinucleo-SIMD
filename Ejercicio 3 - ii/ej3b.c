#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <immintrin.h>
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
void operacion(cuaternions *dp, cuaternions *a, cuaternions *b);


//
int main(int argc, char *argv[])
{
	FILE *archivo = fopen(argv[1], "a"); // archivo donde guardaremos los datos
	
	// Variables
	cuaternions *a, *b; // Vectores de cuaterniones iniciales, con los que se opera
	cuaternions dp; // Cuaternion final, resultado de las operaciones
	double ck; // Contador de ciclos
	
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

	/* OPERACIONES */
	// Operacion de los cuaterniones: producto y suma, con vectorizacion de iteraciones
	operacion(&dp, a, b);

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


/* Se realizan todas las operaciones (adiciones y productos) con los dos vectores de cuaterniones en un solo bucle, con procesamiento vectorial
 * Se vectoriza las iteraciones del bucle (multiplicacion de cuatro pares de cuaterniones a cada iteracion)
 *  - Argumentos: cuaternion resultado, vector de cuaterniones, vector de cuaterniones
 *  - Resultado: ... <= los cambios se realizan directamente sobre los parametros
 */
void operacion(cuaternions *dp, cuaternions *a, cuaternions *b)
{
	// Se reserva memoria como un bloque alineado para un cuaternion auxiliar, en forma de vector, no de estructura
	double *c = _mm_malloc(4 * CUAT * sizeof(double), 32);

	__m256d a1, a2, a3, a4, b1, b2, b3, b4; // Vectores de AVX que almacenaran los valores de los cuaterniones con los que se va a operar, en este caso cada vector contiene todos los componentes con el mismo indice de los cuatro cuaterniones, p.e.: a1 contiene las componentes 1 de los cuatro cuaterniones de a
	__m256d com1, com2, com3, com4; // Vectores de AVX que almacenaran los valores tras la primera multiplicacion, siguen estructurandose de la misma manera, cada cuaternion contiene componentes del mismo tipo de los cuatro cuaterniones con los que se opera
	__m256d c1, c2, c3, c4; // Vectores de AVX que almacenaran los valores de la multiplicacion c*c, sigue estructurandose de la misma manera
	__m256d dp_aux = _mm256_set_pd(0, 0, 0, 0); // Contendra el valor del cuaternion final, resultado de las operaciones, en forma de vector

	// Las operaciones se realizan con cuatros cuaterniones simultaneamente, guardando cada componente (0-3) en un vector avx y operando con ello
	for (int i = 0; i < N; i += 4)
	{
		// Se pasan los cuaterniones con los que se van a operar de estructura a vector de AVX, se meten en orden inverso en el set
		a1 = _mm256_set_pd(a[i + 3].c1, a[i + 2].c1, a[i + 1].c1, a[i].c1);
		a2 = _mm256_set_pd(a[i + 3].c2, a[i + 2].c2, a[i + 1].c2, a[i].c2);
		a3 = _mm256_set_pd(a[i + 3].c3, a[i + 2].c3, a[i + 1].c3, a[i].c3);
		a4 = _mm256_set_pd(a[i + 3].c4, a[i + 2].c4, a[i + 1].c4, a[i].c4);
		b1 = _mm256_set_pd(b[i + 3].c1, b[i + 2].c1, b[i + 1].c1, b[i].c1);
		b2 = _mm256_set_pd(b[i + 3].c2, b[i + 2].c2, b[i + 1].c2, b[i].c2);
		b3 = _mm256_set_pd(b[i + 3].c3, b[i + 2].c3, b[i + 1].c3, b[i].c3);
		b4 = _mm256_set_pd(b[i + 3].c4, b[i + 2].c4, b[i + 1].c4, b[i].c4);

		/* MULTIPLICACION a*b */
		// Primero, se calcula el producto de cada componente de un cuaternion con el otro, dando como resultado otrs cuatro cuaterniones, cada uno con componentes del mismo indice (hay que tener muy en cuenta los signos con 'fmsub')
		com1 = _mm256_fmsub_pd(a1, b1, _mm256_fmadd_pd(a2, b2, _mm256_fmadd_pd(a3, b3, _mm256_mul_pd(a4, b4))));
		com2 = _mm256_fmadd_pd(a1, b2, _mm256_fmadd_pd(a2, b1, _mm256_fmsub_pd(a3, b4, _mm256_mul_pd(a4, b3))));
		com3 = _mm256_fmadd_pd(a1, b3, _mm256_fmadd_pd(a3, b1, _mm256_fmsub_pd(a4, b2, _mm256_mul_pd(a2, b4))));
		com4 = _mm256_fmadd_pd(a1, b4, _mm256_fmsub_pd(a2, b3, _mm256_fmsub_pd(a3, b2, _mm256_mul_pd(a4, b1))));

		/* MULTIPLICACION c*c */
		// Despues, se calcula el producto sobre si mismo: c*c, con el mismo procedimiento que el anterior (lo mismo que antes con los signos con 'fmsub')
		c1 = _mm256_fmsub_pd(com1, com1, _mm256_fmadd_pd(com2, com2, _mm256_fmadd_pd(com3, com3, _mm256_mul_pd(com4, com4))));
		c2 = _mm256_fmadd_pd(com1, com2, _mm256_fmadd_pd(com2, com1, _mm256_fmsub_pd(com3, com4, _mm256_mul_pd(com4, com3))));
		c3 = _mm256_fmadd_pd(com1, com3, _mm256_fmadd_pd(com3, com1, _mm256_fmsub_pd(com4, com2, _mm256_mul_pd(com2, com4))));
		c4 = _mm256_fmadd_pd(com1, com4, _mm256_fmsub_pd(com2, com3, _mm256_fmsub_pd(com3, com2, _mm256_mul_pd(com4, com1))));
			
		/* ADICION a dp */
		// Al final, se suman todas las componentes del  mismo tipo (0-3), y el resultado se acumula al valor anterior, resultando en dp (forma de vector AVX), corresponde a: dp+=dp+c*c
// Mascaras:                                                                                         |11011000                                            |11011000
		dp_aux = _mm256_add_pd(dp_aux, _mm256_hadd_pd(_mm256_permute4x64_pd(_mm256_hadd_pd(c1, c3), 216), _mm256_permute4x64_pd(_mm256_hadd_pd(c2, c4), 216)));
	}
	//Se guarda el valor de dp acumulado en c, pasando del tipo de dato de AVX (vecto de 4 doubles) a un vector normal de 4 doubles
	_mm256_store_pd(c, dp_aux);

	// Y se asigna el valor final de dp (guardado en forma de vector en c) a la estructura del cuaternion
	dp->c1 = c[0], dp->c2 = c[1], dp->c3 = c[2], dp->c4 = c[3];

	// Liberacion de memoria
	_mm_free(c);
}
