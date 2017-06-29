CFLAGS=-std=gnu99 -g -O2 -lm -Wall

.PHONY: clean 

deea: deea.c 

clean:
	rm -f deea *~
