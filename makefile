CXX=g++
CXXFLAGS=-Wall -g
LDFLAGS=-lz -lpng
DEPS=datastream.hpp tagparser.hpp level.hpp coord.hpp chunk.hpp render.hpp nbtconstants.hpp minecraft.hpp colorfile.hpp platform.hpp makefile
OBJS=platform.o datastream.o tagparser.o chunk.o level.o minecraft.o colorfile.o render.o cnbt.o
TARGET=cnbt

%.o: %.cpp $(DEPS)
	$(CXX) -c -o $@ $< $(CXXFLAGS)

$(TARGET): $(OBJS)
	g++ -o $@ $^ $(CXXFLAGS) $(LDFLAGS)


.PHONY: clean

clean:
	rm -f $(OBJS)
	rm -f $(TARGET)
