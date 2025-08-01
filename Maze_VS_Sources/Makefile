default_build: compile link launch

compile: main.cpp elements.cpp Maze.cpp Settings.cpp structures.cpp technical.cpp elements.h Maze.h Settings.h technical.h SFML-3.0.0-linux-gcc-64-bit/SFML-3.0.0
	g++ -c main.cpp elements.cpp Maze.cpp Settings.cpp structures.cpp technical.cpp -I SFML-3.0.0-linux-gcc-64-bit/SFML-3.0.0/include -lsfml-graphics -lsfml-window -lsfml-system

link: elements.o main.o Maze.o Settings.o structures.o technical.o
	g++ -o app elements.o main.o Maze.o Settings.o structures.o technical.o -L SFML-3.0.0-linux-gcc-64-bit/SFML-3.0.0/lib -lsfml-graphics -lsfml-window -lsfml-system

launch:
	export LD_LIBRARY_PATH=SFML-3.0.0-linux-gcc-64-bit/SFML-3.0.0/lib && ./app
clean:
	rm *.o app
add:
	git status
	git add *.cpp *.hpp Makefile
	git status
