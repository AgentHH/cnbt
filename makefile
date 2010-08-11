CXX=g++
CXXFLAGS=-Wall -std=c++0x
LDFLAGS=-lz
DEPS=datastream.hpp tagparser.hpp level.hpp nbtconstants.hpp
OBJ=datastream.o tagparser.o level.o main.o
TARGET=cnbt

%.o: %.cpp $(DEPS)
	$(CXX) -c -o $@ $< $(CXXFLAGS)

$(TARGET): $(OBJ)
	g++ -o $@ $^ $(CXXFLAGS) $(LDFLAGS)

.PHONY: clean

clean:
	rm -f $(OBJ)
	rm -f $(TARGET)
