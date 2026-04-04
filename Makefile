CC = gcc
EXE = game

CFLAGS = -lSDL2 -lgifenc -lgifdec -g -I. -I lecram/gifenc -I lecram/gifdec
LDFLAGS = -L. -L lecram/gifenc -L lecram/gifdec
LDLIBS = -std=c11 -g

OBJS = main.o utils.o canvas.o mouse.o palette.o 

all: game

game: $(OBJS)
	$(CC) -o $(EXE) $(LDFLAGS) $(OBJS) $(CFLAGS) $(LDLIBS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm game
	rm $(OBJS)


