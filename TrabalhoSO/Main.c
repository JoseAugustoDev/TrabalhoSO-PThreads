#pragma once
#define _CRT_SECURE_NO_WARNINGS 1
#define _WINSOCK_DEPRECATED_NO_WARNINGS 1
#pragma comment(lib,"pthreadVC2.lib")
#define HAVE_STRUCT_TIMESPEC
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#define INTERVALO_MATRIZ 31999
#define TAMANHO_MATRIZ 10000 // 22000 foi o maior que consegui rodar na minha maquina
#define TAMANHO_MACRO_BLOCO 2
#define NUM_THREADS 6

int** matriz;
int contador = 0;

pthread_mutex_t mutex;

int ehPrimo(int n);

int** alocar_matriz(int linhas, int colunas) {
	int** matriz;
	int i;

	if (linhas < 1 || colunas < 1) {
		printf("Tamanho invalido para a matriz\n");
		return (NULL);
	}

	matriz = calloc(linhas, sizeof(int*));

	if (matriz == NULL) {
		printf("Erro ao alocar memoria para a matriz\n");
		return (NULL);
	}

	for (i = 0; i < linhas; i++)
	{
		matriz[i] = calloc(colunas, sizeof(int));
		if (matriz[i] == NULL) {
			printf("Erro ao alocar memoria para a matriz\n");
			return (NULL);
		}
	}

	return (matriz);
}

int **liberar_matriz(int linhas, int colunas, int** matriz) {
	int i;
	if (matriz == NULL) {
		return (NULL);
	}

	if (linhas < 1 || colunas < 1) {
		printf("Erro: Parametro Invalido\n");
		return (matriz);
	}

	for (i = 0; i < linhas; i++) free(matriz[i]);
	free(matriz);

	return (NULL);
}

double buscaSerial(int *contador_serial) {
	clock_t inicio, fim;

	int cont = 0;

	inicio = clock();
	for (int i = 0; i < TAMANHO_MATRIZ; i++) {
		for (int j = 0; j < TAMANHO_MATRIZ; j++) {
			if (ehPrimo(matriz[i][j])) {
				cont++;
			}
		}
	}
	fim = clock();

	*contador_serial = cont;

	return (double)(fim - inicio) / CLOCKS_PER_SEC;
}

void* runner(void* param) {

	(void)param;

	int contador_local = 0;

	while (1) {

		pthread_mutex_lock(&mutex);
		contador += contador_local;
		pthread_mutex_unlock(&mutex);
	}

	return NULL;
}

double buscaParalela(int num_threads, int *contador_paralelo) {
	clock_t inicio, fim;

	pthread_t workers[NUM_THREADS];

	pthread_mutex_init(&mutex, NULL);

	inicio = clock();

	for (int i = 0; i < NUM_THREADS; i++)
	{
		pthread_t tid;
		pthread_create(&tid, NULL, runner, NULL);
	}

	for (int i = 0; i < NUM_THREADS; i++)
	{
		pthread_join(workers[i], NULL);
	}

	fim = clock();

	pthread_mutex_destroy(&mutex);

	*contador_paralelo = contador;
	return (double)(fim - inicio) / CLOCKS_PER_SEC;
}

void* runner(void* param);

int main(int argc, char* argv[]) {

	int** matriz;

	matriz = alocar_matriz(TAMANHO_MATRIZ, TAMANHO_MATRIZ);
	if (!matriz) return (1);

	srand(time(NULL));

	// Preenchendo matriz com numeros aleatorios
	for (int i = 0; i < TAMANHO_MATRIZ; i++) {
		for (int j = 0; j < TAMANHO_MATRIZ; j++) {
			matriz[i][j] = rand() % INTERVALO_MATRIZ + 1;
		}
	}

	int contador_serial = 0;
	double tempo_serial = buscaSerial(&contador_serial);

	printf("Total de numeros primos (serial): %d\n", contador_serial);
	printf("Tempo: %.5f", tempo_serial);

	int contador_paralelo = 0;
	double tempo_paralelo = buscaParalela(NUM_THREADS, &contador_paralelo);
	double speedup = tempo_serial / tempo_paralelo;

	printf("Quantidade de threads: %d\n", NUM_THREADS);
	printf("\nTotal de numeros primos (paralelo): %d\n", contador_paralelo);
	printf("Tempo: %.5f\n", tempo_paralelo);
	printf("Speedup: %.5f\n", speedup);

	matriz = liberar_matriz(TAMANHO_MATRIZ, TAMANHO_MATRIZ, matriz);

	return 0;
}

int ehPrimo(int n) {

	if (n < 2) return 0;
	if (n == 2) return 1;
	if (n % 2 == 0) return 0;

	int sqrt_n = (int)floor(sqrt(n));

	for (int i = 3; i <= sqrt_n; i+=2)
	{
		if	(n % i == 0) {
			return 0;
		}
	}

	return 1;
}
