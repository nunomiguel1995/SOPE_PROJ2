CC = gcc

CFLAGS= -Wall

RM = rm -f

all: parque.o

parque.o: parque.c 
		$(CC) $(CFLAGS) -o bin/parque parque.c -D_REENTRANT -lpthread
		 
clean:  
		$(RM) bin/*