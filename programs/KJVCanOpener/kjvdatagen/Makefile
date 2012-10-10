CXX = g++
LD = g++
CXXFLAGS = -Wall -W -pedantic -g -ggdb -I/cygdrive/c/Boost/boost_1_51_0 -c -o
LDFLAGS = -o $(EXEC)
EXEC = kjvdatagen
OBJECTS = kjvdatagen.o

#-include $(OBJECTS:.o=.d)

all: $(OBJECTS)
	$(LD) $(OBJECTS) $(LDFLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $@ $<

#	$(CXX) $(CXXFLAGS) $@ $< -MMD -MP

clean:
	rm -f $(OBJECTS) $(EXEC)

