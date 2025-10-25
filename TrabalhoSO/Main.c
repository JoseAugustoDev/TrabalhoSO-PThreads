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
#define TAMANHO_MACRO_BLOCO 3

int** matriz;
int contador = 0;

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

int main(int argc, char* argv[]) {

	int** matriz;

	matriz = alocar_matriz(TAMANHO_MATRIZ, TAMANHO_MATRIZ);

	srand(time(NULL));

	clock_t start_t, end_t;
	double total_t;

	for (int i = 0; i < TAMANHO_MATRIZ; i++) {
		for (int j = 0; j < TAMANHO_MATRIZ; j++) {
			matriz[i][j] = rand() % INTERVALO_MATRIZ + 1;
		}
	}

	
	start_t = clock();
	for (int i = 0; i < TAMANHO_MATRIZ; i++)
	{
		for (int j = 0; j < TAMANHO_MATRIZ; j++)
		{
			if (ehPrimo(matriz[i][j])) {
				contador++;
			}
		}
		
	}
	end_t = clock();

	total_t = (double)(end_t - start_t) / CLOCKS_PER_SEC;

	printf("Total de numeros primos: %d\n", contador);
	printf("Tempo CPU: %f\n", total_t);

	matriz = liberar_matriz(TAMANHO_MATRIZ, TAMANHO_MATRIZ, matriz);

	return 0;
}

int ehPrimo(int n) {

	if (n < 2) return 0;
	if (n == 2) return 1;
	if (n % 2 == 0) return 0;

	int sqrt_n = sqrt(n);

	for (int i = 3; i <= sqrt_n; i+=2)
	{
		if	(n % i == 0) {
			return 0;
		}
	}

	return 1;
}