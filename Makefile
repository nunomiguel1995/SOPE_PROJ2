CC = gcc

CFLAGS= -Wall

RM = rm -f

all: parque.o gerador.o

parque.o: parque.c 
		$(CC) $(CFLAGS) -o bin/parque parque.c -D_REENTRANT -lpthread
gerador.o: gerador.c
		$(CC) $(CFLAGS) -o bin/gerador gerador.c -D_REENTRANT -lpthread	 
clean:  
		$(RM) bin/*