CC = gcc
EXE = game

CFLAGS = -lSDL2 -lgifenc -lgifdec -g -I. -I lecram/gifenc -I lecram/gifdec
LDFLAGS = -L. -L lecram/gifenc -L lecram/gifdec
LDLIBS = -std=c11 -lm -g

OBJS = main.o utils.o canvas.o mouse.o palette.o button.o scrollbar.o

all: game

game: $(OBJS)
	$(CC) -o $(EXE) $(OBJS) $(CFLAGS) $(LDFLAGS) $(LDLIBS)

%.o: %.c
	$(CC) -c $< -o $@ $(LDFLAGS) $(CFLAGS)

clean:
	rm game
	rm $(OBJS)


