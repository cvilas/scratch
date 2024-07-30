
#LINUX
COMMON=-O2 -I /home/vilas/programs/include -I../../include -L /home/vilas/programs/lib -L /home/vilas/programs/lib64 -L../../lib -pthread -Wl,-no-as-needed -Wl,-rpath,'$$ORIGIN'/../../lib
LIBS = -lmujoco -lglfw -lm
CC = gcc


ROOT = main

all:
	$(CC) $(COMMON) main.c $(LIBS) -o $(ROOT)

main.o:
	$(CC) $(COMMON) -c main.c

clean:
	rm *.o $(ROOT)
