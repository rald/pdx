all: game

game: main.c types.h canvas.c canvas.h
	gcc main.c canvas.c -o game -std=c11 -I. -L. -I lecram/gifenc -L lecram/gifenc -I lecram/gifdec -L lecram/gifdec -lSDL2 -lgifenc -lgifdec -g

clean:
	rm game

