TARGET = main
LIBS = -lm
CC = mpiCC
CFLAGS = -g3 -Wall -pedantic -std=c++11

.PHONY: default all clean

default: $(TARGET)
all: default
run: $(TARGET)
	-$(TARGET) 9 10 20 3 graf.txt

OBJECTS = $(patsubst %.cpp, %.o, $(wildcard *.cpp))
HEADERS = $(wildcard *.h)

%.o: %.cpp $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

.PRECIOUS: $(TARGET) $(OBJECTS)

$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) $(CFLAGS) $(LIBS) -o $@

clean:
	-rm -f *.o
	-rm -f $(TARGET)