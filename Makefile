CC=gcc
singer: singer.c 
	$(CC) singer.c -o singer -lX11 -lm -lasound -lpthread
sing1: sing1.c 
	$(CC) -Og sing1.c -o sing1 -lX11 -lm -lasound -lpthread
all: singer  sing1
