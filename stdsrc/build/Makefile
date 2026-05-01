CXX = g++
CXXFLAGS = -std=c++11 -O2 -Wall

TARGET = ../../bin/build
OBJS = main.o

all: $(TARGET)

$(TARGET): $(OBJS)
	mkdir -p ../../bin
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS)

main.o: main.cpp
	$(CXX) $(CXXFLAGS) -c main.cpp

clean:
	rm -f $(TARGET) $(OBJS)
