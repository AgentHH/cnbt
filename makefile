CXX=g++
CXXFLAGS=-Wall -O2
LDFLAGS=-lz
DEPS=datastream.hpp tagparser.hpp level.hpp coord.hpp chunk.hpp render.hpp nbtconstants.hpp
OBJ=datastream.o tagparser.o chunk.o level.o render.o main.o
TARGET=cnbt

%.o: %.cpp $(DEPS)
	$(CXX) -c -o $@ $< $(CXXFLAGS)

$(TARGET): $(OBJ)
	g++ -o $@ $^ $(CXXFLAGS) $(LDFLAGS)

.PHONY: clean

clean:
	rm -f $(OBJ)
	rm -f $(TARGET)
