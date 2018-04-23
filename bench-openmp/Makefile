
CC=gcc
CFLAGS=-fopenmp -std=c99 -O3 -DNDEBUG -Wall
#CFLAGS=-fopenmp -std=c99 -g -Wall
LDFLAGS=-fopenmp

#CC=icc
#CFLAGS=-qopenmp -qopenmp-link=static -std=c99
#LDFLAGS=-qopenmp -qopenmp-link=static

LDLIBS=-lm
ARTIFACTS=integrate heat fft


all: $(ARTIFACTS)

%: %.c

clean:
	rm -f $(ARTIFACTS) *.o
