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

#define TAMANHO_NOME_FIFO		5
#define TAMANHO_FIFO_PRIVADO 	1000
#define FIFO_NORTE				"/temp/fifoN"
#define FIFO_SUL				"/temp/fifoS"
#define FIFO_ESTE				"/temp/fifoE"
#define FIFO_OESTE				"/temp/fifoO"

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
int id_veiculo = 1, ticks = 0;
FILE *log_gerador;

typedef enum {ENTRA, SAI, CHEIO} EstadoParque;

typedef struct{
	char *direcao;
	int id;
	int t_estacionamento;
	char fifo_entrada[TAMANHO_NOME_FIFO];
	char fifo_privado[TAMANHO_FIFO_PRIVADO];
}Veiculo;

void *enviaVeiculo(void *veiculo){
	Veiculo *v = (Veiculo *)veiculo;
	int fifo, fifoPrivado;
	char *estadoParque = malloc(sizeof(char));

	fifo = open(v->fifo_entrada, O_WRONLY);
	write(fifo, v, sizeof(Veiculo));

	fifoPrivado = open(v->fifo_privado, O_RDONLY);
	//read(fifo, estadoParque, sizeof(estadoParque));

	fprintf(log_gerador, "%5d    ;  %4d   ; %4s   ; %6d     ; %4d   ;  \n", ticks, v->id, v->direcao, v->t_estacionamento, 0);

	close(fifo);

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
	int intervalo;

	v->id = id_veiculo;
	id_veiculo++;
	v->direcao = gerarEntrada();

	strcpy(v->fifo_entrada, "fifo");
	strcat(v->fifo_entrada, v->direcao);
	v->t_estacionamento = gerarTempoEstacionamento(relogio);
	sprintf(v->fifo_privado, "%s%d", "viatura",v->id);

	pthread_create(&thr_enviaVeiculo, NULL, enviaVeiculo, v);
	pthread_detach(thr_enviaVeiculo);

	intervalo = gerarIntervaloViaturas(relogio);
	ticks += intervalo;

	return intervalo;
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

	log_gerador = fopen("gerador.log", "w");
	fprintf(log_gerador, "t(ticks) ; id_viat ; destin ; t_estacion ; t_vida ; observ\n");

	do{
		time(&t_atual);

		if(intervalo == 0){
			intervalo = gerarVeiculo(u_relogio);
		}else {
			intervalo--;
		}
		usleep(u_relogio * pow(10,3));
	}while(difftime(t_atual, t_inicial) < t_geracao);

	fclose(log_gerador);

	exit(0);
}
