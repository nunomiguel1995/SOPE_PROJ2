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
#define TAMANHO_BUFFER			75
#define	FIFO					"fifo"
#define FIFO_NORTE				"fifoN"
#define FIFO_SUL				"fifoS"
#define FIFO_ESTE				"fifoE"
#define FIFO_OESTE				"fifoO"

#define V_ENTROU				0
#define V_ESTACIONA				1
#define V_SAIU					2
#define P_ABERTO				3
#define P_CHEIO					4
#define P_FECHADO				5

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
int id_veiculo = 1, ticks;
int log_gerador;
char bufferEstado[TAMANHO_BUFFER];
char buffer[TAMANHO_BUFFER];

typedef struct{
	char direcao;
	char fifo_viatura[TAMANHO_BUFFER];
	char fifo_entrada[TAMANHO_NOME_FIFO];
	int id;
	int t_estacionamento;
	int ticks_criacao;
	int ticks_entradaParque;
}Veiculo;

void escreveLog(Veiculo *v, int estado){
	switch(estado){
	case P_FECHADO:
		strcpy(bufferEstado, "encerrado");
		break;
	case P_CHEIO:
		strcpy(bufferEstado, "cheio!");
		break;
	case V_SAIU:
		strcpy(bufferEstado, "saida");
		break;
	case V_ENTROU:
		strcpy(bufferEstado, "estacionamento");
		break;
	case V_ESTACIONA:
		strcpy(bufferEstado, "estacionamento");
		break;
	default:
		break;
	}

	if(estado == V_SAIU){
		sprintf(buffer,"%5d    ;  %4d   ; %4c   ; %6d     ; %4d   ;  %s\n", (v->ticks_criacao + v->t_estacionamento), v->id, v->direcao, v->t_estacionamento, (ticks - v->ticks_criacao + v->t_estacionamento),bufferEstado);
	}else{
		sprintf(buffer,"%5d    ;  %4d   ; %4c   ; %6d     ;    ?   ;  %s\n", v->ticks_criacao, v->id, v->direcao, v->t_estacionamento, bufferEstado);
	}

	if(write(log_gerador, buffer, strlen(buffer)) == -1){
		perror("write:: Erro ao escrever no gerador.log\n");
	}
	strcpy(buffer, "");
	strcpy(bufferEstado, "");
}

void *enviaVeiculo(void *veiculo){
	pthread_detach(pthread_self());
	Veiculo v = *(Veiculo *)veiculo;
	int fifoEscreve, fifoLe, estado = 0;

	if(mkfifo(v.fifo_viatura, 0660) == -1){
		perror("mkfifo:: Erro ao criar fifo.\n");
	}

	fifoEscreve = open(v.fifo_entrada, O_WRONLY | O_NONBLOCK);

	if(fifoEscreve != -1){
		if(write(fifoEscreve, &v, sizeof(Veiculo)) == -1){
			perror("write:: Erro ao escrever para o controlador.\n");
		}
		close(fifoEscreve);

		fifoLe = open(v.fifo_viatura, O_RDONLY);
		if(fifoLe != -1){
			if(read(fifoLe, &estado, sizeof(int)) == -1){
				perror("read:: Erro ao ler do arrumador.\n");
			}

			pthread_mutex_lock(&mutex);
			escreveLog(&v, estado);
			pthread_mutex_unlock(&mutex);

			if(read(fifoLe, &estado, sizeof(int)) == -1){
				perror("read:: Erro ao ler do arrumador.\n");
			}
			close(fifoLe);
		}
	}else{
		estado = P_FECHADO;
	}

	pthread_mutex_lock(&mutex);
	escreveLog(&v, estado);
	pthread_mutex_unlock(&mutex);

	unlink(v.fifo_viatura);

	return NULL;
}

char gerarEntrada(){
	int random = rand() % 4;

	if(random == 0){
		return 'N';
	}else if(random == 1){
		return 'S';
	}else if(random == 2){
		return 'E';
	}else{
		return 'O';
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
	char buffer[TAMANHO_BUFFER];

	v->id = id_veiculo;
	v->direcao = gerarEntrada();
	v->t_estacionamento = gerarTempoEstacionamento(relogio);
	v->ticks_criacao = ticks;

	switch(v->direcao){
	case 'N':
		strcpy(v->fifo_entrada, "fifoN");
		break;
	case 'S':
		strcpy(v->fifo_entrada, "fifoS");
		break;
	case 'O':
		strcpy(v->fifo_entrada, "fifoO");
		break;
	case 'E':
		strcpy(v->fifo_entrada, "fifoE");
		break;
	default:
		break;
	}

	sprintf(buffer, "%s%d", FIFO, v->id);
	strcpy(v->fifo_viatura, buffer);

	intervalo = gerarIntervaloViaturas(relogio);
	ticks+=intervalo;
	id_veiculo++;

	if(pthread_create(&thr_enviaVeiculo, NULL, enviaVeiculo, v) != 0){
		perror("Gerador:: Erro a criar thread 'viatura'.");
	}

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

	ticks = 0;

	log_gerador = open("gerador.log",O_RDONLY | O_WRONLY | O_TRUNC, 0660);
	if(log_gerador == -1){
		perror("open:: Erro ao abrir o gerador.log\n");
		exit(1);
	}
	char buffer[100] = "t(ticks) ; id_viat ; destin ; t_estacion ; t_vida ; observ\n";
	write(log_gerador, buffer, strlen(buffer));

	do{
		time(&t_atual);
		if(intervalo == 0){
			intervalo = gerarVeiculo(u_relogio);
		}else {
			intervalo--;
		}
		usleep(u_relogio * pow(10,3));
		time(&t_atual);
	}while(difftime(t_atual, t_inicial) < t_geracao);

	close(log_gerador);

	exit(0);
}
