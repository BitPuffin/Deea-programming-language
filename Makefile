CFLAGS=-std=gnu99 -g -O2 -Wall

.PHONY: clean 

deea: deea.c

clean:
	rm -f deea *~
