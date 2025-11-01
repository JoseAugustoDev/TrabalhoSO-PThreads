//Autores José Augusto e Letícia Comissário da Silva

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

//----------------------------------------------------------------------------------------------------------------------//
#define INTERVALO_MATRIZ 31999
#define TAMANHO_MATRIZ 10000
#define TAMANHO_MACRO_BLOCO 500 
#define NUM_THREADS 300
//----------------------------------------------------------------------------------------------------------------------//
int blocoAtual = 0; //variavel global para controlar o bloco atual
int** matriz; 
int contador=0;
pthread_mutex_t mutex;
//----------------------------------------------------------------------------------------------------------------------//
int ehPrimo(int n);

int** Alocar_matriz_int(int m, int n)
{
	int** v;
	int i;
	//Verifica se os parametros sao validos
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
	//Verifica se os parametros sao validos
	if (m < 1 || n < 1) {
		printf("** Erro: Parametro invalido **\n");
		return v;
	}
	//Libera a memoria alocada
	for (i = 0; i < m; i++) free(v[i]);
	free(v);
	return NULL;
}

void* runner(void* param) {
	(void)param;
	int threadId = *(int*)param;//codigo para pegar o id da thread (cada uma tem um identificador unico)
	int contador_local=0;//contador para cada thread
	int blocoLinha=TAMANHO_MATRIZ/TAMANHO_MACRO_BLOCO;//qtd de blocos na linha
	int blocoColuna=TAMANHO_MATRIZ/TAMANHO_MACRO_BLOCO;//qtd de blocos na coluna
	int totalBlocos=blocoLinha*blocoColuna;//qtd total de blocos

	while (1) {
		int bloco;
		pthread_mutex_lock(&mutex);
		bloco = blocoAtual;     // pega o número do próximo bloco
		blocoAtual = blocoAtual + 1;           // avança para o próximo bloco
		pthread_mutex_unlock(&mutex);
		if (bloco >= totalBlocos) {
			//verifica se ja acabou todos os blocos
			break;
		}
		int linhaInicio=(bloco/blocoColuna)*TAMANHO_MACRO_BLOCO;//calcula a linha inicial do bloco
		int colunaInicio=(bloco%blocoColuna)*TAMANHO_MACRO_BLOCO;//calcula a coluna inicial do bloc
		//zera o contador porque cada bloco tem seu proprio 
		contador_local=0;

		//calcula o fim do bloco
		int linhaFim=linhaInicio+TAMANHO_MACRO_BLOCO;
		//verifica se o fim do bloco ultrapassa o tamanho da matriz
		if (linhaFim>TAMANHO_MATRIZ) {
			linhaFim=TAMANHO_MATRIZ;
		}
		int colunaFim=colunaInicio+TAMANHO_MACRO_BLOCO;
		//verifica se o fim do bloco ultrapassa o tamanho da matriz
		if (colunaFim>TAMANHO_MATRIZ) {
			colunaFim=TAMANHO_MATRIZ;
		}

		//Percorre o macro bloco e conta os primos
		for (int i = linhaInicio; i < linhaFim; i++) {
			for (int j = colunaInicio; j < colunaFim; j++) {
				if (ehPrimo(matriz[i][j])) {
					contador_local++;
				}
			}
		}
		//atualiza o contador global
		pthread_mutex_lock(&mutex);//trava o mutex evita condicao de corrida
		contador += contador_local;//atualiza o contador 
		pthread_mutex_unlock(&mutex);//destrava o mutex
	}

	return NULL;
}

double buscaParalela(int* contador_paralelo) {
	clock_t inicio, fim;
	// Cria as threads
	pthread_t workers[NUM_THREADS];
	//array para armazenar os IDs das threads
	int thread_ids[NUM_THREADS];
	// Inicializa o mutex
	pthread_mutex_init(&mutex, NULL);

	inicio = clock();
	// Cria as threads para processar os blocos
	for (int i = 0; i < NUM_THREADS; i++)
	{
		thread_ids[i] = i;
		//Verifica se a thread foi criada e se tem algum erro
		if (pthread_create(&workers[i], NULL, runner, &thread_ids[i]) != 0) {
			perror("Erro ao criar a thread");
			exit(1);
		}
	}

	for (int i = 0; i < NUM_THREADS; i++)
	{
		//join para esperar todas as threads terminarem
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
	// Percorre a matriz inteira contando os numeros primos
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
	//definir a semente
	srand(time(NULL));

	printf("Preenchendo matriz %dx%d com numeros aleatorios de 1 a %d\n", TAMANHO_MATRIZ, TAMANHO_MATRIZ, INTERVALO_MATRIZ);

	// Preenchendo matriz com numeros aleatorios
	for (int i = 0; i < TAMANHO_MATRIZ; i++) {
		for (int j = 0; j < TAMANHO_MATRIZ; j++) {
			matriz[i][j] = rand() % INTERVALO_MATRIZ + 1;
		}
	}

	printf("\n");

	printf("Iniciando busca serial\n");

	int contador_serial=0;
	double tempo_serial = buscaSerial(&contador_serial);

	printf("Total de numeros primos (serial): %d\n", contador_serial);
	printf("Tempo: %.5f\n", tempo_serial);

	printf("\nIniciando busca paralelo\n");

	int contador_paralelo = 0;
	double tempo_paralelo = buscaParalela(&contador_paralelo);
	double speedup = tempo_serial/tempo_paralelo;

	printf("\nQuantidade de threads: %d\n", NUM_THREADS);
	printf("Total de numeros primos (paralelo): %d\n", contador_paralelo);
	printf("Tempo: %.5f\n", tempo_paralelo);
	printf("Speedup: %.5f\n", speedup);

	matriz = Liberar_matriz_int(TAMANHO_MATRIZ, TAMANHO_MATRIZ, matriz);

	return 0;
}

int ehPrimo(int n) {
	// Retorna 1 se n for primo, 0 caso contrario
	//arrumei ele quando o 2 aparecia falava que não era primo
	//dividi o if
	if (n<2){
		return 0;
	}
	if (n==2){
		return 1;
	}
	if (n%2==0) {
		return 0;
	}
	// Testa divisores impares de 3 a sqrt(n) (calcula a raiz)
	int sqrt_n = (int)floor(sqrt(n));
	//Loop para verificar se n é divisível por algum número ímpar até a raiz quadrada de n
	for (int i = 3; i <= sqrt_n; i += 2)
	{
		if (n % i == 0) {
			return 0;
		}
	}
	return 1;
}
