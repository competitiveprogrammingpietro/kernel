.PHONY: all
all: build blank.bin blank.elf 

build: 
	rm -rf build && mkdir build

blank.bin: blank.o
	sed 's/OUTPUT_FORMAT(placeholder)/OUTPUT_FORMAT(binary)/' < ./linker.ld \
		> build/linker.bin.ld
	i686-elf-gcc -g -T ./build/linker.bin.ld \
			-o ./build/blank.bin \
			-ffreestanding \
			-O0 -nostdlib -fpic -g \
			./build/blank.o

blank.elf: blank.o
	sed 's/OUTPUT_FORMAT(placeholder)/OUTPUT_FORMAT(elf32-i386)/' < ./linker.ld \
		> build/linker.elf.ld
	i686-elf-gcc -g -T ./build/linker.elf.ld \
			-o ./build/blank.elf \
			-ffreestanding \
			-O0 -nostdlib -fpic -g \
			./build/blank.o

blank.o: blank.asm
	nasm -f elf ./blank.asm -o ./build/blank.o

%.o: %.c
	i686-elf-gcc $(INCLUDES) $(FLAGS) -c $< -o build/$@ -std=gnu99

%.asm.o: %.asm
	nasm -f elf -g $< -o build/$@
clean:
	rm -rf ./build

.PHONY=clean build