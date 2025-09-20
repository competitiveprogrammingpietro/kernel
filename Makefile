# Boot with gdb  target remote | qemu-system-x84_64 -hda boot.bin -S -gdb stdio
FILES = ./build/kernel.asm.o
all: ./bin/boot.bin  ./bin./kernel.bin
	dd if=./bin/boot.bin > ./bin/os.bin
	dd if=./bin/kernel.bin >> ./bin/os.bin # It might not take a whole sector
	dd if=/dev/zero bs=512 count=100 >> ./bin/os.bin

./bin/boot.bin: ./src/boot/boot.asm
	nasm -f bin src/boot/boot.asm -o bin/boot.bin 

./bin./kernel.bin: $(FILES)
	i686-elf-ld -g -relocatable $(FILES) -o ./build/kernelfull.o
	i686-elf-gcc -T ./src/linker.ld -o ./bin/kernel.bin -ffreestanding -O0 -nostdlib ./build/kernelfull.o

./build/kernel.asm.o: ./src/kernel.asm
	nasm -f elf -g ./src/kernel.asm -o ./build/kernel.asm.o

clean:
	rm -rf bin/boot/boot.bin ./bin/os.bin


# Create a bootable binary hard drive with two sector, the first contains our
# bootloader while the second contains same data
# qemu-system-x86_64 -hda boot.bin
# This is all material useful to learn about the real mode
bootloader_disk_io:
	nasm -f bin assembly/bootloader_load_from_disk.asm -o boot_sample.bin
	dd if=assembly/message.txt >> boot_sample.bin
	dd if=/dev/zero bs=512 count=1 >> boot_sample.bin
