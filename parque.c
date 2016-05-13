#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>
#include <time.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>

typedef struct{
	int n_lugares; //lotação
	int t_abertura; //período de tempo, em segundos, em que o Parque está aberto
}Parque;

Parque parque;
time_t t_inicial;
pthread_t thr_entradaN, thr_entradaS, thr_entradaE, thr_entradaO;

void criaThreads(){
	pthread_create(&thr_entradaN, NULL, entrada_parque, "N");
	pthread_create(&thr_entradaS, NULL, entrada_parque, "S");
	pthread_create(&thr_entradaE, NULL, entrada_parque, "E");
	pthread_create(&thr_entradaO, NULL, entrada_parque, "O");
}

void esperaThreads(){
	pthread_join(thr_entradaN,NULL);
	pthread_join(thr_entradaS,NULL);
	pthread_join(thr_entradaE,NULL);
	pthread_join(thr_entradaO,NULL);
}

void *entrada_parque(void *arg){
	char* entrada = (char* )arg;

	char nome_fifo[5] = "fifo";
	strcat(nome_fifo, entrada);
	mkfifo(nome_fifo, 0666);

	printf("%s\n", nome_fifo);

	pthread_exit(NULL);
}

void *inicializaParque(void *arg){
	Parque* p = (Parque*)arg;
	time_t t_atual;

	t_inicial = time(NULL);
	parque.n_lugares = p->n_lugares;
	parque.t_abertura = p->t_abertura;

	printf("O parque abriu.\n");
	do{
		time(&t_atual);
		criaThreads();

		esperaThreads();
	}while(difftime(t_atual, t_inicial) < parque.t_abertura);
	printf("O parque fechou.\n");

	pthread_exit(NULL);
}

int main(int argc, char *argv[]){
	pthread_t thr_principal; //thread principal
	Parque p;

	if (argc != 3){
		fprintf(stderr, "Usage: %s dir_name\n", argv[0]);
		exit(1);
	}

	p.n_lugares = atoi(argv[1]);
	p.t_abertura = atoi(argv[2]);

	pthread_create(&thr_principal, NULL, inicializaParque, (void *)&p);
	pthread_join(thr_principal,NULL);

	exit(0);
}
