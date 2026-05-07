.PHONY: all
STDLIB=../../stdlib/stdlib.elf
INCLUDES=-I../../stdlib/src
FLAGS=-O0 -g
all: build/mainc.elf

build/mainc.elf: build/mainc.o
	i686-elf-gcc -g -T ./linker.ld \
			-o ./build/mainc.elf \
			-ffreestanding \
			-O0 -nostdlib -fpic \
			-g ./build/mainc.o $(STDLIB)

build/%.o: %.c
	mkdir -p $(@D)
	i686-elf-gcc $(INCLUDES) -I./$(FLAGS) -std=gnu99 -c $^ -o $@

clean:
	rm -rf ./build

.PHONY=clean build