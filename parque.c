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
#define TAMANHO_NOME_FIFO	5
#define TAMANHO_FIFO_PRIVADO 	1000

typedef enum {ENTRA, SAI, CHEIO} EstadoParque;

typedef struct{
	int n_lugares; //lotação
	int t_abertura; //período de tempo, em segundos, em que o Parque está aberto
	int aberto; //controlador de abertura de parque
}Parque;

typedef struct{
	char *direcao;
	int id;
	int t_estacionamento;
	char nome_fifo[TAMANHO_NOME_FIFO];
	char fifo_privado[TAMANHO_FIFO_PRIVADO];
}Veiculo;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_t thr_Norte, thr_Sul, thr_Este, thr_Oeste;
Parque parque;
int numVagasParque;
FILE *log_parque;

void *arrumador(void *veiculo){

	Veiculo *v = (Veiculo *)veiculo;
	int fifoVeiculo;

	fifoVeiculo = open(v->fifo_privado, O_WRONLY);

	//pthread_mutex_lock(&mutex);

	if(numVagasParque > 0 && parque.aberto){
		EstadoParque estado = ENTRA;
		numVagasParque--;
		write(fifoVeiculo, &estado, sizeof(EstadoParque));
	}

	//pthread_mutex_unlock(&mutex);

	pthread_exit(NULL);
}

void *entradaParqueNorte(void *arg){
	mkfifo(FIFO_NORTE, 0650);
	int fifo, openF = 1, readF;
	Veiculo v;

	fifo = open(FIFO_NORTE, O_RDONLY | O_NONBLOCK);

	while(openF){
		readF = read(fifo, &v, sizeof(Veiculo));
		if(v.id == -1){
			openF = 0;
		}else if(readF > 0){
			printf("%2d %6s %9d\n", v.id, "Norte", v.t_estacionamento);
		}
	}

	printf("Entrada Norte fechou.\n");
	close(fifo);

	pthread_exit(NULL);
}

void *entradaParqueSul(void *arg){
	mkfifo(FIFO_SUL, 0650);
	int fifo, openF = 1, readF;
	Veiculo v;

	fifo = open(FIFO_SUL, O_RDONLY | O_NONBLOCK);

	while(openF){
		readF = read(fifo, &v, sizeof(Veiculo));
		if(v.id == -1){
			openF = 0;
		}else if(readF > 0){
			printf("%2d %6s %9d\n", v.id, "Sul", v.t_estacionamento);
		}
	}

	printf("Entrada Sul fechou.\n");
	close(fifo);

	pthread_exit(NULL);
}

void *entradaParqueEste(void *arg){
	mkfifo(FIFO_ESTE, 0650);
	int fifo, openF = 1, readF;
	Veiculo v;

	fifo = open(FIFO_ESTE, O_RDONLY | O_NONBLOCK);

	while(openF){
		readF = read(fifo, &v, sizeof(Veiculo));
		if(v.id == -1){
			openF = 0;
		}else if(readF > 0){
			printf("%2d %6s %9d\n", v.id, "Este", v.t_estacionamento);
		}
	}

	printf("Entrada Este fechou.\n");
	close(fifo);

	pthread_exit(NULL);
}

void *entradaParqueOeste(void *arg){
	mkfifo(FIFO_OESTE, 0650);
	int fifo, openF = 1, readF;
	Veiculo v;

	fifo = open(FIFO_OESTE, O_RDONLY | O_NONBLOCK);

	while(openF){
		readF = read(fifo, &v, sizeof(Veiculo));
		if(v.id == -1){
			openF = 0;
		}else if(readF > 0){
			printf("%2d %6s %9d\n", v.id, "Oeste", v.t_estacionamento);
		}
	}

	printf("Entrada Oeste fechou.\n");
	close(fifo);

	pthread_exit(NULL);
}

void *criarControladores(void *arg){
	pthread_create(&thr_Norte, NULL, entradaParqueNorte, NULL);
	pthread_create(&thr_Sul, NULL, entradaParqueSul, NULL);
	pthread_create(&thr_Este, NULL, entradaParqueEste, NULL);
	pthread_create(&thr_Oeste, NULL, entradaParqueOeste, NULL);

	return NULL;
}

void enviaUltimoVeiculo(){
	int fifoN, fifoS, fifoE, fifoO;
	Veiculo v;
	v.id = -1;

	fifoN = open(FIFO_NORTE, O_WRONLY);
	fifoS = open(FIFO_SUL, O_WRONLY);
	fifoE = open(FIFO_ESTE, O_WRONLY);
	fifoO = open(FIFO_OESTE, O_WRONLY);

	write(fifoN, &v, sizeof(Veiculo));
	write(fifoS, &v, sizeof(Veiculo));
	write(fifoE, &v, sizeof(Veiculo));
	write(fifoO, &v, sizeof(Veiculo));
}

void fechaControladores(){
	pthread_join(thr_Norte, NULL);
	pthread_join(thr_Sul, NULL);
	pthread_join(thr_Este, NULL);
	pthread_join(thr_Oeste, NULL);
}

void apagaFifos(){
	unlink(FIFO_NORTE);
	unlink(FIFO_SUL);
	unlink(FIFO_ESTE);
	unlink(FIFO_OESTE);
}

int main(int argc, char *argv[]){
	pthread_t thr_principal;

	if(argc != 3){
		perror("Numero errado de argumentos. Utilize ./parque <N_LUGARES> <T_ABERTURA>\n");
		exit(1);
	}
	parque.n_lugares = atoi(argv[1]);
	parque.t_abertura = atoi(argv[2]);
	parque.aberto = 1;

	pthread_create(&thr_principal, NULL, criarControladores, NULL);

	sleep(parque.t_abertura);
	parque.aberto = 0;

	enviaUltimoVeiculo();
	fechaControladores();

	apagaFifos();

	exit(0);
}
