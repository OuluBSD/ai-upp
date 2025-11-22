# Makefile for API extractor tool

CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2
TARGET = api_extractor
SOURCES = api_extractor.cpp

all: $(TARGET)

$(TARGET): $(SOURCES)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SOURCES)

clean:
	rm -f $(TARGET)

install: $(TARGET)
	cp $(TARGET) ../..

.PHONY: all clean install