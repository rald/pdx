# file: Makefile

CC = gcc
EXE = game

CFLAGS = -lSDL2 -lgifenc -lgifdec -g -I. -I lecram/gifenc -I lecram/gifdec
LDFLAGS = -L. -L lecram/gifenc -L lecram/gifdec
LDLIBS = -std=c89 -lm -g

OBJS = main.o utils.o canvas.o mouse.o target.o palette.o button.o scrollbar.o MyWindow.o

all: libgifenc.a libgifdec.a game

libgifenc.a: lecram/gifenc/gifenc.c lecram/gifenc/gifenc.h
	$(CC) -c lecram/gifenc/gifenc.c -o lecram/gifenc/gifenc.o
	ar rcs lecram/gifenc/libgifenc.a lecram/gifenc/gifenc.o

libgifdec.a: lecram/gifdec/gifdec.c lecram/gifdec/gifdec.h
	$(CC) -c lecram/gifdec/gifdec.c -o lecram/gifdec/gifdec.o
	ar rcs lecram/gifdec/libgifdec.a lecram/gifdec/gifdec.o

game: $(OBJS)
	$(CC) -o $(EXE) $(OBJS) $(CFLAGS) $(LDFLAGS) $(LDLIBS)

%.o: %.c
	$(CC) -c $< -o $@ $(LDFLAGS) $(CFLAGS) $(LDLIBS)

clean:
	rm game
	rm $(OBJS)
	rm lecram/gifenc/libgifenc.a lecram/gifenc/gifenc.o
	rm lecram/gifdec/libgifdec.a lecram/gifdec/gifdec.o
