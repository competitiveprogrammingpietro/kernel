# This is the basis of a debugging session with GDB for a bare metal kernel
# add-auto-load-safe-path /home/ppaolini/kernel/src/kernel/.gdbinit

# Useful defaults for kernel work
#set disassemble-next-line on
#set confirm off
set pagination off
#set print asm-demangle on
set disassembly-flavor intel

add-symbol-file ./build/kernelfull.o 0x100000


break kernel.c:127
break fs.c:66
layout src
target remote | qemu-system-i386 -hda ./bin/os.bin -S -gdb stdio --accel tcg
