CXX=g++
CXXFLAGS=-Wall -O2
LDFLAGS=-lz -lpng
DEPS=datastream.hpp tagparser.hpp level.hpp coord.hpp chunk.hpp render.hpp nbtconstants.hpp minecraft.hpp
OBJS=datastream.o tagparser.o chunk.o level.o minecraft.o render.o cnbt.o
TARGET=cnbt

%.o: %.cpp $(DEPS)
	$(CXX) -c -o $@ $< $(CXXFLAGS)

$(TARGET): $(OBJS)
	g++ -o $@ $^ $(CXXFLAGS) $(LDFLAGS)


.PHONY: clean

clean:
	rm -f $(OBJS)
	rm -f $(TARGET)
