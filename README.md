# kernel. To be executed with:
qemu-system-x86_64 -hda boot.bin

# Debug
gdb target remote | qemu-system-x86_64 -hda boot.bin -S -gdb stdio

# Disassemble
ndisasm program

# Compile the compiler

Following the instruction at:

https://wiki.osdev.org/GCC_Cross-Compiler

Binutils version used 2.35
GCC verion used: 10.2.0

