all:
	rm -f main.o
	gcc -o main.o main.c src/EXT2_Utils.c Shell/Shell.c include/EXT2.h