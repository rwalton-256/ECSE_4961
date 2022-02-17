CC=g++
CFLAGS=-g

ODIR=obj
LDIR =../lib

LIBS=-pthread -lzstd

_OBJ = main.o zstd_comp.o
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))



$(ODIR)/%.o: %.cpp
	mkdir -p $(ODIR)
	$(CC) -c -o $@ $< $(CFLAGS)

zstd_custom: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

.PHONY: clean

clean:
	rm -f $(ODIR)/*.o
