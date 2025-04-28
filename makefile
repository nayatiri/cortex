cortex:
	g++ -std=c++20 -O2 -o build/cortex src/main.cpp src/renderer.cc src/components/*.cc src/glad/glad.c -g -lglfw -ldl -lpthread -lX11 -lXxf86vm -lXrandr -lXi  -lGL

.PHONY: test clean

test: cortex
	./build/cortex

clean:
	rm -f ./build/cortex
