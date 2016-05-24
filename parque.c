/*
 * parque.c
 *      Author: Nuno Castro
 *      Author: Sara Santos
 */
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <errno.h>
#include <time.h>
#include <math.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>

#define FIFO_NORTE				"fifoN"
#define FIFO_SUL				"fifoS"
#define FIFO_ESTE				"fifoE"
#define FIFO_OESTE				"fifoO"
#define TAMANHO_NOME_FIFO		5
#define TAMANHO_BUFFER			75

#define V_ENTROU				0
#define V_ESTACIONA				1
#define V_SAIU					2
#define P_ABERTO				3
#define P_CHEIO					4
#define P_FECHADO				5

typedef struct{
	int n_lugares; //lotação
	int t_abertura; //período de tempo, em segundos, em que o Parque está aberto
	int aberto; //controlador de abertura de parque
}Parque;

typedef struct{
	char direcao;
	char fifo_viatura[TAMANHO_BUFFER];
	char fifo_entrada[TAMANHO_NOME_FIFO];
	int id;
	int t_estacionamento;
	int ticks_criacao;
	int ticks_entradaParque;
}Veiculo;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutexFicheiro = PTHREAD_MUTEX_INITIALIZER;
pthread_t thr_Norte, thr_Sul, thr_Este, thr_Oeste;
Parque parque;
int numVagasParque;
int log_parque;

void escreveLog(Veiculo *v, int estado){
	char buffer[TAMANHO_BUFFER];

	switch(estado){
	case P_FECHADO:
		sprintf(buffer,"%5d    ; %4d   ; %4d    ; encerrado\n", v->ticks_criacao, numVagasParque, v->id);
		break;
	case P_CHEIO:
		sprintf(buffer,"%5d    ; %4d   ; %4d    ; cheio!\n", v->ticks_criacao, numVagasParque, v->id);
		break;
	case V_SAIU:
		if(parque.aberto == 1){
			sprintf(buffer,"%5d    ; %4d   ; %4d    ; saída\n", v->ticks_criacao + v->t_estacionamento, numVagasParque, v->id);
		}else if(parque.aberto == 0){
			sprintf(buffer,"%5d    ; %4d   ; %4d    ; encerrado\n", v->ticks_criacao + v->t_estacionamento, numVagasParque, v->id);
		}
		break;
	case V_ENTROU:
		sprintf(buffer,"%5d    ; %4d   ; %4d    ; entrada\n", v->ticks_criacao, numVagasParque, v->id);
		break;
	case V_ESTACIONA:
		sprintf(buffer,"%5d    ; %4d   ; %4d    ; estacionamento\n", v->ticks_criacao, numVagasParque, v->id);
		break;
	default:
		break;
	}

	write(log_parque, buffer, strlen(buffer));
	strcpy(buffer, "");
}

void *arrumador(void *veiculo){
	Veiculo *v = (Veiculo *)veiculo;
	int fifoVeiculo, estado;

	fifoVeiculo = open(v->fifo_viatura, O_WRONLY);

	pthread_mutex_lock(&mutex);
	if(parque.aberto == P_ABERTO && numVagasParque > 0){
		estado = V_ENTROU;
		numVagasParque--;
		write(fifoVeiculo, &estado, sizeof(int));
		pthread_mutex_unlock(&mutex);

		pthread_mutex_lock(&mutexFicheiro);
		escreveLog(v, estado);
		pthread_mutex_unlock(&mutexFicheiro);

		usleep(v->t_estacionamento * pow(10,3));
		numVagasParque++;
		estado = V_SAIU;
	}else if(parque.aberto == P_FECHADO){
		pthread_mutex_unlock(&mutex);
		estado = P_FECHADO;
	}else{
		pthread_mutex_unlock(&mutex);
		estado = P_CHEIO;
	}

	write(fifoVeiculo, &estado, sizeof(int));

	pthread_mutex_lock(&mutexFicheiro);
	escreveLog(v, estado);
	pthread_mutex_unlock(&mutexFicheiro);

	close(fifoVeiculo);

	return NULL;
}

void *entradaParqueNorte(void *arg){
	int fifo, readF;
	pthread_t thr_arrumador;
	Veiculo *v = (Veiculo *)malloc(sizeof(Veiculo));

	fifo = open(FIFO_NORTE, O_RDONLY);

	while(parque.aberto == P_ABERTO){
		readF = read(fifo, v, sizeof(Veiculo));
		if(v->id == -1){
			break;
		}else if(readF > 0){
			pthread_create(&thr_arrumador, NULL, arrumador, v);
		}
	}

	close(fifo);

	return NULL;
}

void *entradaParqueSul(void *arg){
	int fifo, readF;
	pthread_t thr_arrumador;
	Veiculo *v = (Veiculo *)malloc(sizeof(Veiculo));

	fifo = open(FIFO_SUL, O_RDONLY);

	while(parque.aberto == P_ABERTO){
		readF = read(fifo, v, sizeof(Veiculo));
		if(v->id == -1){
			break;
		}else if(readF > 0){
			pthread_create(&thr_arrumador, NULL, arrumador, v);
		}
	}

	close(fifo);

	return NULL;
}

void *entradaParqueEste(void *arg){
	int fifo, readF;
	pthread_t thr_arrumador;
	Veiculo *v = (Veiculo *)malloc(sizeof(Veiculo));

	fifo = open(FIFO_ESTE, O_RDONLY);

	while(parque.aberto == P_ABERTO){
		readF = read(fifo, v, sizeof(Veiculo));
		if(v->id == -1){
			break;
		}else if(readF > 0){
			pthread_create(&thr_arrumador, NULL, arrumador, v);
		}
	}

	close(fifo);

	return NULL;
}

void *entradaParqueOeste(void *arg){
	int fifo, readF;
	pthread_t thr_arrumador;
	Veiculo *v = (Veiculo *)malloc(sizeof(Veiculo));

	fifo = open(FIFO_OESTE, O_RDONLY);

	while(parque.aberto == P_ABERTO){
		readF = read(fifo, v, sizeof(Veiculo));
		if(v->id == -1){
			break;
		}else if(readF > 0){
			pthread_create(&thr_arrumador, NULL, arrumador, v);
		}
	}

	close(fifo);

	return NULL;
}

void criarControladores(){
	pthread_create(&thr_Norte, NULL, entradaParqueNorte, NULL);
	pthread_create(&thr_Sul, NULL, entradaParqueSul, NULL);
	pthread_create(&thr_Este, NULL, entradaParqueEste, NULL);
	pthread_create(&thr_Oeste, NULL, entradaParqueOeste, NULL);

	mkfifo(FIFO_NORTE, 0660);
	mkfifo(FIFO_SUL, 0660);
	mkfifo(FIFO_ESTE, 0660);
	mkfifo(FIFO_OESTE, 0660);
}

void fechaControladores(){
	pthread_join(thr_Norte, NULL);
	pthread_join(thr_Sul, NULL);
	pthread_join(thr_Este, NULL);
	pthread_join(thr_Oeste, NULL);

	unlink(FIFO_NORTE);
	unlink(FIFO_SUL);
	unlink(FIFO_ESTE);
	unlink(FIFO_OESTE);
}

int main(int argc, char *argv[]){
	int fifoN, fifoS, fifoE, fifoO;

	if(argc != 3){
		perror("Numero errado de argumentos. Utilize ./parque <N_LUGARES> <T_ABERTURA>\n");
		exit(1);
	}

	parque.n_lugares = atoi(argv[1]);
	numVagasParque = atoi(argv[1]);
	parque.t_abertura = atoi(argv[2]);
	parque.aberto = P_ABERTO;

	Veiculo v;
	v.id = -1;
	v.t_estacionamento = 0;
	strcpy(v.fifo_entrada,"fim");

	log_parque = open("parque.log", O_WRONLY | O_CREAT, 0660);
	char buffer[100] = "t(ticks) ; nlug   ; id_viat ; observ\n";
	write(log_parque, buffer, strlen(buffer));

	criarControladores();

	sleep(parque.t_abertura);
	parque.aberto = P_FECHADO;

	fifoN = open(FIFO_NORTE, O_WRONLY);
	fifoS = open(FIFO_SUL, O_WRONLY);
	fifoE = open(FIFO_ESTE, O_WRONLY);
	fifoO = open(FIFO_OESTE, O_WRONLY);
	write(fifoN, &v, sizeof(Veiculo));
	write(fifoS, &v, sizeof(Veiculo));
	write(fifoE, &v, sizeof(Veiculo));
	write(fifoO, &v, sizeof(Veiculo));
	close(fifoN);
	close(fifoS);
	close(fifoE);
	close(fifoO);

	fechaControladores();

	close(log_parque);

	pthread_exit(NULL);
}
