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
#define q 7 // Decide el valor de N, probar con valores: 2, 4, 6, 7

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
	// Operacion de los cuaterniones: producto y suma, con vectorizacion
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
 * Se vectoriza la multiplizacion de cuaterniones (multiplicacion de un par de cuaterniones a cada iteracion)
 *  - Argumentos: cuaternion resultado, vector de cuaterniones, vector de cuaterniones
 *  - Resultado: ... <= los cambios se realizan directamente sobre los parametros
 */
void operacion(cuaternions *dp, cuaternions *a, cuaternions *b)
{
	// Se reserva memoria como un bloque alineado para un cuaternion auxiliar, en forma de vector, no de estructura
	double *c = _mm_malloc(sizeof(cuaternions), 32);

	// Datos AVX
	__m256d aux_a, aux_b; // Vectores de AVX que almacenaran los valores de los cuaterniones con los que se va a operar
	__m256d aux_c; // Vector de AVX que almacenara el valor de la primera multiplicacion
	__m256d columna1, columna2, columna3, columna4; // Almacena los valores de cada columna de la multiplicacion
	__m256d signos; // Vector que ira almacenando unidades y positivas para realizar cambios de signo de algunos datos
	__m256d dp_aux = _mm256_set_pd(0, 0, 0, 0); // Contendra el valor del cuaternion final, resultado de las operaciones, en forma de vector

	// Las operaciones se realizan por columnas (de lo que se entiende como multiplicacion de cuaterniones)
	for (int i = 0; i < N; i++)
	{
		// Se pasan los cuaterniones con los que se van a operar de estructura a vector de AVX, se meten en orden inverso en el set
		aux_a = _mm256_set_pd(a[i].c4, a[i].c3, a[i].c2, a[i].c1);
		aux_b = _mm256_set_pd(b[i].c4, b[i].c3, b[i].c2, b[i].c1);
		// Para conseguir las diferentes operaciones, se permutaran los vectores anteriores para conseguir la secuencia adecuada

		/* MULTIPLICACION a*b */
		// Multiplicacion a*b: PRIMERA COLUMNA
		// Se establecen los nuevos cambios de signo para la multiplicacion: ya son todos positivos, no se usa el vector
// Mascara:                                                     |00000000
		columna1 = _mm256_mul_pd(_mm256_permute4x64_pd(aux_a, 0), aux_b); // Multiplicacion primera columna

		// Multiplicacion a*b: SEGUNDA COLUMNA
		signos = _mm256_set_pd(1, -1, 1, -1); // Se establecen los nuevos cambios de signo para la multiplicacion (orden inverso)
// Mascaras:                                                                          |01010101                         |10110001
		columna2 = _mm256_mul_pd(signos, _mm256_mul_pd(_mm256_permute4x64_pd(aux_a, 85), _mm256_permute4x64_pd(aux_b, 177))); // Multiplicacion segunda columna

		// Multiplicacion a*b: TERCERA COLUMNA
		signos = _mm256_set_pd(-1, 1, 1, -1); // Se establecen los nuevos cambios de signo para la multiplicacion (orden inverso)
// Mascaras:                                                                          |10101010                          |01001110
		columna3 = _mm256_mul_pd(signos, _mm256_mul_pd(_mm256_permute4x64_pd(aux_a, 170), _mm256_permute4x64_pd(aux_b, 78))); // Multiplicacion tercera columna

		// Multiplicacion a*b: CUARTA COLUMNA
		signos = _mm256_set_pd(1, 1, -1, -1); // Se establecen los nuevos cambios de signo para la multiplicacion (orden inverso)
// Mascaras:                                                                          |11111111                          |00011011
		columna4 = _mm256_mul_pd(signos, _mm256_mul_pd(_mm256_permute4x64_pd(aux_a, 255), _mm256_permute4x64_pd(aux_b, 27))); // Multiplicacion cuarta columna

		// Al final, se suman las cuatro columnas, resultando en el cuaternion c (en forma de vector AVX), que corresponde la multiplicacion de a*b
		aux_c = _mm256_add_pd(_mm256_add_pd(columna1, columna2), _mm256_add_pd(columna3, columna4));

		/* MULTIPLICACION c*c */
		// Multiplicacion c*c: PRIMERA COLUMNA
		// Se establecen los nuevos cambios de signo para la multiplicacion: ya son todos positivos, no se usa el vector
// Mascara:                                                     |00000000
		columna1 = _mm256_mul_pd(_mm256_permute4x64_pd(aux_c, 0), aux_c); // Multiplicacion primera columna

		// Multiplicacion c*c: SEGUNDA COLUMNA
		signos = _mm256_set_pd(1, -1, 1, -1); // Se establecen los nuevos cambios de signo para la multiplicacion (orden inverso)
// Mascaras:                                                                          |01010101                         |10110001
		columna2 = _mm256_mul_pd(signos, _mm256_mul_pd(_mm256_permute4x64_pd(aux_c, 85), _mm256_permute4x64_pd(aux_c, 177))); // Multiplicacion segunda columna

		// Multiplicacion c*c: TERCERA COLUMNA
		signos = _mm256_set_pd(-1, 1, 1, -1); // Se establecen los nuevos cambios de signo para la multiplicacion (orden inverso)
// Mascaras:                                                                          |10101010                          |01001110
		columna3 = _mm256_mul_pd(signos, _mm256_mul_pd(_mm256_permute4x64_pd(aux_c, 170), _mm256_permute4x64_pd(aux_c, 78))); // Multiplicacion tercera columna

		// Multiplicacion c*c: CUARTA COLUMNA
		signos = _mm256_set_pd(1, 1, -1, -1); // Se establecen los nuevos cambios de signo para la multiplicacion (orden inverso)
// Mascaras:                                                                          |11111111                          |00011011
		columna4 = _mm256_mul_pd(signos, _mm256_mul_pd(_mm256_permute4x64_pd(aux_c, 255), _mm256_permute4x64_pd(aux_c, 27))); // Multiplicacion cuarta columna

		/* ADICION a dp */
		// Al final, se suman las cuatro columnas, y el resultado se acumula al valor que se tenia, resultando en el cuaternion dp (en forma de vector AVX), corresponde a: dp+=dp+c*c
		dp_aux = _mm256_add_pd(dp_aux, _mm256_add_pd(_mm256_add_pd(columna1, columna2), _mm256_add_pd(columna3, columna4)));
	}
	
	//Se guarda el valor de dp acumulado en c, pasando del tipo de dato de AVX (vecto de 4 doubles) a un vector normal de 4 doubles
	_mm256_store_pd(c, dp_aux);

	// Y se asigna el valor final de dp (guardado en forma de vector en c) a la estructura del cuaternion
	dp->c1 = c[0], dp->c2 = c[1], dp->c3 = c[2], dp->c4 = c[3];

	// Liberacion de memoria
	_mm_free(c);
}
