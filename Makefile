# Boot with gdb  target remote | qemu-system-x84_64 -hda boot.bin -S -gdb stdio
all:
	nasm -f bin src/boot/boot.asm -o bin/boot/boot.bin 

clean:
	rm -rf bin/boot/boot.bin


# Create a bootable binary hard drive with two sector, the first contains our
# bootloader while the second contains same data
# qemu-system-x86_64 -hda boot.bin
# This is all material useful to learn about the real mode
bootloader_disk_io:
	nasm -f bin assembly/bootloader_load_from_disk.asm -o boot_sample.bin
	dd if=assembly/message.txt >> boot_sample.bin
	dd if=/dev/zero bs=512 count=1 >> boot_sample.bin
