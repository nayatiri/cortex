# Makefile

# Compiler
CXX = g++

# Compiler flags
CXXFLAGS = -std=c++20 -O2 -g

# Source files
SOURCES = glad/glad.c components/*.cc renderer.cc main.cpp  

# Object files (replace .cc with .o)
OBJECTS = $(SOURCES:.cc=.o)
OBJECTS := $(OBJECTS:.c=.o)  # Also replace .c with .o for glad.c

# Executable name
EXECUTABLE = app

# Default target
all: $(EXECUTABLE)

# Link object files to create the executable
$(EXECUTABLE): $(OBJECTS)
	$(CXX) $(OBJECTS) -o $@ -lglfw -ldl -lpthread -lX11 -lXxf86vm -lXrandr -lXi -lasound -lGL -lassimp

# Compile source files to object files
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

%.o: %.cc
	$(CXX) $(CXXFLAGS) -c $< -o $@

%.o: %.c
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean up build files
.PHONY: all test clean

test: all
	./$(EXECUTABLE)

clean:
	rm -f $(OBJECTS) $(EXECUTABLE)
