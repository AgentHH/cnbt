CXX=g++
CXXFLAGS=-Wall
LDFLAGS=-lz
DEPS=datastream.hpp tagparser.hpp constants.hpp
OBJ=datastream.o tagparser.o main.o
TARGET=cnbt

%.o: %.cpp $(DEPS)
	$(CXX) -c -o $@ $< $(CXXFLAGS)

$(TARGET): $(OBJ)
	g++ -o $@ $^ $(CXXFLAGS) $(LDFLAGS)

.PHONY: clean

clean:
	rm -f $(OBJ)
	rm -f $(TARGET)
