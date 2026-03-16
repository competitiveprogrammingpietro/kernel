.PHONY: all
all: 
	rm -rf build && mkdir build
	nasm -f elf ./blank.asm -o ./build/blank.o
	i686-elf-gcc -g -T ./linker.ld -o ./build/blank.bin -ffreestanding -O0 -nostdlib -fpic -g ./build/blank.o

clean:
	rm -rf ./build