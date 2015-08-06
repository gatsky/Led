CC=g++
CFLAGS=-std=c++0x -Wall -c
LDFLAGS=-lpthread
SOURCES=LedClient.cpp Client.cpp
OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE=LedClient

all: $(SOURCES) $(EXECUTABLE)
    
$(EXECUTABLE): $(OBJECTS) 
	$(CC) $(OBJECTS) -o $@ $(LDFLAGS) 

.cpp.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -rf *.o $(EXECUTABLE)
