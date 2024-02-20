#===========================Makefile================================#
CC = gcc

all: oss worker

oss: oss.c
	$(CC) -o oss oss.c

worker: worker.c
	$(CC) -o worker worker.c

clean:
	rm -f oss worker