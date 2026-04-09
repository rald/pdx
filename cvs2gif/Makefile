CC = gcc

all: cvs2gif

cvs2gif: cvs2gif.c
	$(CC) cvs2gif.c -o cvs2gif -I ../lecram/gifenc -L ../lecram/gifenc -lSDL2 -lgifenc -g

clean:
	rm cvs2gif

