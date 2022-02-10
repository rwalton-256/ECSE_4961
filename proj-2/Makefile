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
	rm -f huge_file.dat
	rm -f comp_file.zst
	rm -f uncomp_file.dat

huge_file.dat:
	head -c 10000000000 /dev/random > huge_file.dat

run: huge_file.dat
	./zstd_custom huge_file.dat comp_file.zst
	unzstd comp_file.zst -o uncomp_file.dat
	diff huge_file.dat uncomp_file.dat
