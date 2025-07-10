CXX = g++
CXXFLAGS = -std=c++20 -O0 -g -Wall -Werror
LDFLAGS = -lglfw -ldl -lpthread -lX11 -lXxf86vm -lXrandr -lXi -lGL
SRC = src/main.cpp src/components/logging.hh src/renderer.cc src/components/*.cc src/glad/glad.c 
TARGET = build/cortex

.PHONY: test clean

# Rule to build the target
$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

# Rule to run tests
test: $(TARGET)
	./$(TARGET)

# Rule to clean up build artifacts
clean:
	rm -f $(TARGET)
