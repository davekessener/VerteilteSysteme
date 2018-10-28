CC=g++
CFLAGS=-O3 -ggdb -Wall
LIBS=-lcaf_core -lcaf_io
SRC=$(wildcard *.cc)
OBJ=$(SRC:.cc=.o)
DEP=$(wildcard *.h)
TARGET=lab2

.PHONY: all run clean

%.o: %.cc $(DEP)
	$(CC) -c $< $(CFLAGS) -o $@

all: $(TARGET)

run: all
	./$(TARGET)

$(TARGET): $(OBJ)
	$(CC) -o $(TARGET) $(OBJ) $(LIBS)

clean:
	rm -f *.o $(TARGET)

