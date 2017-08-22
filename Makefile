CFLAGS=-std=gnu99 -g -lm -Wall -I.
CC=gcc

OBJ=main.o 

.PHONY: clean 

deea: $(OBJ) 
		$(CC) $(OBJ) -o deea $(CFLAGS)

clean:
	rm -f deea *~
