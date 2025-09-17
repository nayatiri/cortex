CXX = g++
CXXFLAGS = -std=c++20 -O0 -g -Wall -Werror
LDFLAGS = -lglfw -ldl -lpthread -lX11 -lXxf86vm -lXrandr -lXi -lGL

# Source files
SRC = src/main.cpp src/renderer.cc src/glad/glad.c $(wildcard src/components/*.cc)

# Object files
OBJ = $(SRC:.cpp=.o)
OBJ := $(OBJ:.cc=.o)
OBJ := $(OBJ:.c=.o)

# Target
TARGET = build/cortex

.PHONY: all test clean

all: $(TARGET)

# Link step
$(TARGET): $(OBJ) | build
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

# Compile step for .cpp and .cc
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

%.o: %.cc
	$(CXX) $(CXXFLAGS) -c $< -o $@

%.o: %.c
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Make sure build dir exists
build:
	mkdir -p build

# Run program
test: $(TARGET)
	./$(TARGET)

# Clean build artifacts
clean:
	rm -rf $(OBJ) $(TARGET)
