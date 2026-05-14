.PHONY: all
STDLIB=../../stdlib/stdlib.elf
INCLUDES=-I../../stdlib/src
FLAGS=-O0 -g  -ffreestanding -falign-jumps -falign-functions -falign-labels -falign-loops -fstrength-reduce -fomit-frame-pointer -finline-functions -Wno-unused-function -fno-builtin -Werror -Wno-unused-label -Wno-cpp -Wno-unused-parameter -nostdlib -nostartfiles -nodefaultlibs -Wall -Iinc
all: build/shell.elf

build/shell.elf: build/shell.o
	i686-elf-gcc -g -T ./linker.ld \
			-o $@ \
			-ffreestanding \
			-O0 -nostdlib -fpic \
			-g $^ $(STDLIB)

build/%.o: %.c
	mkdir -p $(@D)
	i686-elf-gcc $(INCLUDES) -I./$(FLAGS) -std=gnu99 -c $^ -o $@

clean:
	rm -rf ./build

.PHONY=clean build