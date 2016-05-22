/*
 * gerador.c
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

#define TAMANHO_NOME_FIFO	5
#define FIFO_NORTE	"fifoN"
#define FIFO_SUL	"fifoS"
#define FIFO_ESTE	"fifoE"
#define FIFO_OESTE	"fifoO"

pthread_mutex_t thr_principal = PTHREAD_MUTEX_INITIALIZER;
int id_veiculo = 1;

typedef struct{
	char *direcao;
	int id;
	int t_estacionamento;
	char nome_fifo[TAMANHO_NOME_FIFO];
}Veiculo;

void *enviaVeiculo(void *veiculo){
	pthread_mutex_lock(&thr_principal);
	Veiculo *v = (Veiculo *)veiculo;
	int fifo;

	//printf("%2d %6s %9d %9s\n", v->id, v->direcao, v->t_estacionamento, v->nome_fifo);
	fifo = open(v->nome_fifo, O_WRONLY);
	write(fifo, v, sizeof(Veiculo));

	close(fifo);

	pthread_mutex_unlock(&thr_principal);
	pthread_exit(NULL);
}

char* gerarEntrada(){
	int random = rand() % 4;

	if(random == 0){
		return "N";
	}else if(random == 1){
		return "S";
	}else if(random == 2){
		return "E";
	}else{
		return "O";
	}
}

int gerarTempoEstacionamento(int relogio){
	return (rand() % 10 + 1) * relogio;
}

int gerarIntervaloViaturas(int relogio){
	int random = rand() % 10;

	if(random < 5) return (0 * relogio);
	else if(random >= 5 && random < 8) return (1 * relogio);
	else return (2 * relogio);
}

int gerarVeiculo(int relogio){
	pthread_t thr_enviaVeiculo;
	Veiculo *v = (Veiculo *)malloc(sizeof(Veiculo));

	v->id = id_veiculo;
	v->direcao = gerarEntrada();

	strcpy(v->nome_fifo, "fifo");
	strcat(v->nome_fifo, v->direcao);
	v->t_estacionamento = gerarTempoEstacionamento(relogio);

	id_veiculo = id_veiculo + 1;

	pthread_create(&thr_enviaVeiculo, NULL, enviaVeiculo, v);
	pthread_join(thr_enviaVeiculo, NULL);


	return gerarIntervaloViaturas(relogio);
}

int main(int argc, char *argv[]){
	srand(time(NULL));
	time_t t_inicial = time(NULL), t_atual;

	if(argc != 3){
		perror("Numero errado de argumentos. Utilize ./gerador <T_GERACAO> <U_RELOGIO>\n");
		exit(1);
	}

	int t_geracao = atoi(argv[1]);
	int u_relogio = atoi(argv[2]);
	int intervalo = 0;

	do{
		time(&t_atual);

		if(intervalo == 0){
			intervalo = gerarVeiculo(u_relogio);
		}else {
			intervalo--;
		}

		usleep(u_relogio * pow(10,3));
	}while(difftime(t_atual, t_inicial) < t_geracao);

	Veiculo *v = (Veiculo *)malloc(sizeof(Veiculo));
	v->id = -1;

	pthread_exit(NULL);
	exit(0);
}
