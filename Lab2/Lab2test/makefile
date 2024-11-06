.SILENT:

build: Myprint.o
	@g++ -o main.out -m32 main.cpp Myprint.o

Myprint.o:
	@nasm -felf32 -o Myprint.o Myprint.asm

run: build
	@./main.out

clean:
	@rm -f *.o
	@rm -f *.out