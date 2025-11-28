# Makefile for THE SHIFTING MAZE
# For Windows with MinGW/MSYS2

CXX = g++
CXXFLAGS = -std=c++11 -Wall -O2
INCLUDES = -I./src
LIBS = -lopengl32 -lglu32 -lfreeglut

TARGET = ShiftingMaze.exe
SRC = src/main.cpp

all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $(TARGET) $(SRC) $(LIBS)

clean:
	del /f $(TARGET) 2>nul || rm -f $(TARGET)

run: $(TARGET)
	./$(TARGET)

.PHONY: all clean run
