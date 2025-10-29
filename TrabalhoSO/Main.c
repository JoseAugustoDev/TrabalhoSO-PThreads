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
#define TAMANHO_MATRIZ 15000
#define TAMANHO_MACRO_BLOCO 250
#define NUM_THREADS 12


int blocoAtual = 0;
int **matriz;
int contador = 0;

pthread_mutex_t mutex;

int ehPrimo(int n);

int** Alocar_matriz_int(int m, int n)
{
	int** v;
	int i;
	if (m < 1 || n < 1) {
		printf("** Erro: Parametro invalido **\n");
		return NULL;
	}

	v = (int**)calloc(m, sizeof(int*));
	if (v == NULL) {
		printf("** Erro: Memoria Insuficiente **\n");
		return NULL;
	}

	for (i = 0; i < m; i++) {
		v[i] = (int*)calloc(n, sizeof(int));
		if (v[i] == NULL) {
			printf("** Erro: Memoria Insuficiente **\n");
			return NULL;
		}
	}

	return v;
}

int** Liberar_matriz_int(int m, int n, int** v)
{
	int i;
	if (v == NULL) return NULL;
	if (m < 1 || n < 1) {
		printf("** Erro: Parametro invalido **\n");
		return v;
	}
	for (i = 0; i < m; i++) free(v[i]);
	free(v);
	return NULL;
}

void* runner(void* param) {
	//não testei poruque ta dando erro no VS e eu to arrumando
	(void)param;
	//depois tirar os print
    int threadId=*(int*)param;//codigo para pegar o id da thread (cada uma tem um identificador unico)
	int contador_local = 0;

	int blocoLinha = TAMANHO_MATRIZ / TAMANHO_MACRO_BLOCO;
	int blocoColuna = TAMANHO_MATRIZ / TAMANHO_MACRO_BLOCO;

	int totalBlocos = blocoLinha * blocoColuna;

	while (1) {
		int bloco;
        pthread_mutex_lock(&mutex);
        bloco=blocoAtual;     // pega o número do próximo bloco
        blocoAtual=blocoAtual+1;           // avança para o próximo bloco
        pthread_mutex_unlock(&mutex);

        //criando os blocos para cada thread trabalhar

        if(bloco>=totalBlocos) {
            //verifica se ja acabou todos os blocos
			/*printf("Thread %d terminou\n", threadId);*/
            break;
        }
        int linhaInicio=(bloco/blocoColuna)*TAMANHO_MACRO_BLOCO;
        int colunaInicio=(bloco%blocoColuna)*TAMANHO_MACRO_BLOCO;
		//só para ver se a divisão ta correta e essas coisas depois tirar isso daqui
		/*printf("Thread %d pegou bloco %d (linha %d a %d, coluna %d a %d)\n",
               threadId,
               bloco,
               linhaInicio,
               linhaInicio+TAMANHO_MACRO_BLOCO-1,
               colunaInicio,
               colunaInicio+TAMANHO_MACRO_BLOCO-1);*/

        contador_local=0;

		int linhaFim = linhaInicio + TAMANHO_MACRO_BLOCO;
		if (linhaFim > TAMANHO_MATRIZ) linhaFim = TAMANHO_MATRIZ;

		int colunaFim = colunaInicio + TAMANHO_MACRO_BLOCO;
		if (colunaFim > TAMANHO_MATRIZ) colunaFim = TAMANHO_MATRIZ;

		//Percorre o macro bloco e conta os primos
		for(int i=linhaInicio;i < linhaFim;i++) {
			for(int j=colunaInicio;j < colunaFim;j++) {
				if(ehPrimo(matriz[i][j])) {
					contador_local++;
				}
			}
		}
		//atualiza o contador global
		pthread_mutex_lock(&mutex);
		contador+=contador_local;
		pthread_mutex_unlock(&mutex);
	}

	return NULL;
}

double buscaParalela(int *contador_paralelo) {
	clock_t inicio, fim;

	pthread_t workers[NUM_THREADS];
	int thread_ids[NUM_THREADS];

	pthread_mutex_init(&mutex, NULL);

	inicio = clock();

	for (int i = 0; i < NUM_THREADS; i++)
	{
		thread_ids[i] = i;
		if (pthread_create(&workers[i], NULL, runner, &thread_ids[i]) != 0) {
			perror("Erro ao criar a thread");
			exit(1);
		}
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


double buscaSerial(int* contador_serial) {
	clock_t inicio, fim;

	int cont = 0;

	inicio = clock();
	for (int i = 0; i < TAMANHO_MATRIZ; i++) {
		for (int j = 0; j < TAMANHO_MATRIZ; j++) {
			if (ehPrimo(matriz[i][j])) cont++;
		}
	}
	fim = clock();

	*contador_serial = cont;

	return (double)(fim - inicio) / CLOCKS_PER_SEC;
}

int main(int argc, char* argv[]) {

	matriz = Alocar_matriz_int(TAMANHO_MATRIZ, TAMANHO_MATRIZ);
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
	double tempo_paralelo = buscaParalela(&contador_paralelo);
	double speedup = tempo_serial / tempo_paralelo;

	printf("\nQuantidade de threads: %d\n", NUM_THREADS);
	printf("Total de numeros primos (paralelo): %d\n", contador_paralelo);
	printf("Tempo: %.5f\n", tempo_paralelo);
	printf("Speedup: %.5f\n", speedup);

	matriz = Liberar_matriz_int(TAMANHO_MATRIZ, TAMANHO_MATRIZ, matriz);

	return 0;
}

int ehPrimo(int n) {

	if (n < 2 || n % 2 == 0) return 0;
	if (n == 2) return 1;

	int sqrt_n = (int)floor(sqrt(n));

	for (int i = 3; i <= sqrt_n; i+=2)
	{
		if	(n % i == 0) {
			return 0;
		}
	}

	return 1;
}
