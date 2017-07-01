CFLAGS=-std=gnu99 -g -O2 -lm -Wall

.PHONY: clean 

deea: deea.c eval.c writer.c reader.c helper.c

clean:
	rm -f deea *~
