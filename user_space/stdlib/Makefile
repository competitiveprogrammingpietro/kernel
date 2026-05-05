all: stdlib.elf

stdlib.elf: ./build/start.asm.o build/runtime.asm.o
	i686-elf-ld -elf_i386 -relocatable $^ -o $@

build/%.o: src/%.c
	@mkdir -p $(@D)
	i686-elf-gcc $(INCLUDES) $(FLAGS) -c $< -o $@ -std=gnu99

build/%.asm.o: src/%.asm
	@mkdir -p $(@D)
	nasm -f elf -g $< -o $@

clean:
	rm -rf ./build
	rm stdlib.elf

.PHONY=clean