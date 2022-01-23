CC=g++
CFLAGS=-mavx2 -march=native -std=c++17 -g -O3 -pthread -funroll-loops

SRC+=matrix_mult.cpp
SRC+=main.cpp

ODIR=obj
LDIR =../lib

LIBS=-lm

_OBJ = main.o 
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))


$(ODIR)/%.o: %.cpp
	$(CC) -c -o $@ $< $(CFLAGS)

matrix_multiplier: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

.PHONY: clean

clean:
	rm -f $(ODIR)/*.o
