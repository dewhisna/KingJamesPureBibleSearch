CXX = g++
LD = g++
CXXFLAGS = -Wall -W -pedantic -g -ggdb -c -o
LDFLAGS = -o $(EXEC)
EXEC = kjvdatagen
OBJECTS = kjvdatagen.o

all: $(OBJECTS)
	$(LD) $(OBJECTS) $(LDFLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $@ $<

clean:
	rm -f $(OBJECTS) $(EXEC)

