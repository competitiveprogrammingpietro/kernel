boot:
	nasm -f bin boot.asm -o boot.bin
	


# Create a bootable binary hard drive with two sector, the first contains our
# bootloader while the second contains same data
# qemu-system-x86_64 -hda boot.bin
# This is all material useful to learn about the real mode
booloader_sample:
	nasm -f bin assembly/bootloader_load_from_disk.asm -o boot_sample.bin
	dd if=message.txt >> boot_sample.bin
	dd if=/dev/zero bs=512 count=1 >> boot_sample.bin
