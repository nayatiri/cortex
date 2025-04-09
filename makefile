cortex: main.cpp
	g++ -std=c++20 -O2 -o app main.cpp renderer.cc  glad/glad.c -g -lglfw -ldl -lpthread -lX11 -lXxf86vm -lXrandr -lXi -lasound -lGL -lassimp

.PHONY: test clean

test: cortex
	./app

clean:
	rm -f app
