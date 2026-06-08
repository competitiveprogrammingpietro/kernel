all: stdlib.elf
FLAGS=-g

stdlib.elf: \
			./build/runtime.asm.o \
			./build/stdlib.o \
			./build/string.o \
			./build/memory.o
	i686-elf-ld -m elf_i386 -relocatable $^ -o $@

build/%.o: src/%.c
	@mkdir -p $(@D)
	i686-elf-gcc $(INCLUDES) $(FLAGS) -c $^ -o $@ -std=gnu99

build/%.asm.o: src/%.asm
	@mkdir -p $(@D)
	nasm -f elf -g $^ -o $@

clean:
	rm -rf ./build
	rm stdlib.elf

.PHONY=clean