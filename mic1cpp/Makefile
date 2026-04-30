CC = g++
CFLAGS = --std=c++20 -Wall -Werror -pedantic -g
LIB = 
# Your .hpp files
DEPS = Control.h Clock.h
# Your compiled .o files
OBJECTS = Control.o Alu.o Clock.o Shifter.o
# The name of your program
PROGRAM = mic1cpp

.PHONY: all clean lint


all: $(PROGRAM)

# Wildcard recipe to make .o files from corresponding .cpp file
%.o: %.cpp $(DEPS)
	$(CC) $(CFLAGS) -c $<

$(PROGRAM): main.o $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^ $(LIB)

clean:
	rm *.o $(PROGRAM) test *.a

