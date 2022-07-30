all: tetris

tetris: src/tetris.cpp
	g++ -g -o ./tetris ./src/tetris.cpp -I./include -lGL -lGLU -lglut
