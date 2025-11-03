# Boot with gdb  target remote | qemu-system-x86_64 -hda boot.bin -S -gdb stdio
# The asm kernel file MUST be the first in the list
#FILES = ./build/kernel.asm.o ./build/kernel.o ./build/idt/idt.asm.o ./build/idt/idt.o ./build/memory/memory.o ./build/io/io.asm.o
FILES = ./build/kernel.asm.o ./build/kernel.o $(shell ls build/*/*.o) $(shell ls build/*/*/*.o)


FILES_C := $(shell find src -name '*.c')
FILES_ASM := $(shell find src -name '*.asm' | grep -v 'boot.asm')

OBJ_C := $(patsubst src/%.c, build/%.o, $(FILES_C))
OBJ_ASM := $(patsubst src/%.asm, build/%.asm.o, $(FILES_ASM))

INCLUDES = -I./src
FLAGS = -g -ffreestanding -falign-jumps -falign-functions -falign-labels -falign-loops -fstrength-reduce -fomit-frame-pointer -finline-functions -Wno-unused-functions -fno-builtin -Werror -Wno-unused-label -Wno-cpp -Wno-unused-parameters -nostdlib -nostartfiles -nodefaultlibs -Wall -O0 -Iinc

all: ./bin/boot.bin  ./bin/kernel.bin
	dd if=./bin/boot.bin > ./bin/os.bin
	dd if=./bin/kernel.bin >> ./bin/os.bin # It might not take a whole sector
	dd if=/dev/zero bs=512 count=100 >> ./bin/os.bin

./bin/boot.bin: ./src/boot/boot.asm
	nasm -f bin src/boot/boot.asm -o bin/boot.bin 

./bin/kernel.bin: $(OBJ_C) $(OBJ_ASM)
	i686-elf-ld -g -relocatable $(FILES) -o ./build/kernelfull.o
	i686-elf-gcc -T ./src/linker.ld -o ./bin/kernel.bin -ffreestanding -O0 -nostdlib ./build/kernelfull.o

./build/kernel.asm.o: ./src/kernel.asm
	nasm -f elf -g ./src/kernel.asm -o ./build/kernel.asm.o

./build/kernel.o: ./src/kernel.c
	i686-elf-gcc $(INCLUDES) $(FLAGS) -c ./src/kernel.c -o ./build/kernel.o -std=gnu99


build/%.o: src/%.c
	@mkdir -p $(@D)
	i686-elf-gcc $(INCLUDES) $(FLAGS) -c $< -o $@ -std=gnu99


build/%.asm.o: src/%.asm
	@mkdir -p $(@D)
	nasm -f elf -g $< -o $@


clean:
	echo $(OBJ_C) $(OBJ_ASM)
	echo damn
	rm -rf ./bin/*.bin
	rm ./build/kernelfull.o $(FILES) 



# Create a bootable binary hard drive with two sector, the first contains our
# bootloader while the second contains same data
# qemu-system-x86_64 -hda boot.bin
# This is all material useful to learn about the real mode
bootloader_disk_io:
	nasm -f bin assembly/bootloader_load_from_disk.asm -o boot_sample.bin
	dd if=assembly/message.txt >> boot_sample.bin
	dd if=/dev/zero bs=512 count=1 >> boot_sample.bin
