CC=gcc
CFLAGS=-I.
DEPS = helper.h
OBJ = crawler.o helper.o 

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

crawler: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)

clean:
	rm -f $(OBJ) $(EXE)