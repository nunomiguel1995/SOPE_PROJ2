/*
 * parque.c
 *      Author: Nuno Castro
 *      Author: Sara Santos
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>
#include <time.h>
#include <math.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>

#define FIFO_NORTE	"fifoN"
#define FIFO_SUL	"fifoS"
#define FIFO_ESTE	"fifoE"
#define FIFO_OESTE	"fifoO"

typedef struct{
	int n_lugares; //lotação
	int t_abertura; //período de tempo, em segundos, em que o Parque está aberto
}Parque;

pthread_mutex_t thr_principal = PTHREAD_MUTEX_INITIALIZER;
pthread_t thr_Norte, thr_Sul, thr_Este, thr_Oeste;
Parque parque;

void *entradaParque(void *arg){
	char* entrada = (char* )arg;

	if(strcmp(entrada,"N") == 0){
		mkfifo(FIFO_NORTE, 0650);
	}else if(strcmp(entrada,"S") == 0){
		mkfifo(FIFO_SUL, 0650);
	}else if(strcmp(entrada,"E") == 0){
		mkfifo(FIFO_ESTE, 0650);
	}else if(strcmp(entrada,"O") == 0){
		mkfifo(FIFO_OESTE, 0650);
	}

	pthread_exit(NULL);
}

int criarControladores(){

	if(pthread_create(&thr_Norte, NULL, entradaParque, "N") != 0){
		perror("Erro ao criar o controlador da entrada Norte.\n");
		exit(1);
	}
	if(pthread_create(&thr_Sul, NULL, entradaParque, "S") != 0){
		perror("Erro ao criar o controlador da entrada Sul.\n");
		exit(1);
	}
	if(pthread_create(&thr_Este, NULL, entradaParque, "E") != 0){
		perror("Erro ao criar o controlador da entrada Este.\n");
		exit(1);
	}
	if(pthread_create(&thr_Oeste, NULL, entradaParque, "O") != 0){
		perror("Erro ao criar o controlador da entrada Oeste.\n");
		exit(1);
	}

	pthread_join(thr_Norte, NULL);
	pthread_join(thr_Sul, NULL);
	pthread_join(thr_Este, NULL);
	pthread_join(thr_Oeste, NULL);

	exit(0);
}

int main(int argc, char *argv[]){
	if(argc != 3){
		perror("Numero errado de argumentos. Utilize ./parque <N_LUGARES> <T_ABERTURA>\n");
		exit(1);
	}
	parque.n_lugares = atoi(argv[1]);
	parque.t_abertura = atoi(argv[2]);

	if(criarControladores() == 1){
		exit(1);
	}

	sleep(parque.t_abertura);

	/*remove(FIFO_NORTE);
	remove(FIFO_SUL);
	remove(FIFO_ESTE);
	remove(FIFO_OESTE);*/

	exit(0);
}
