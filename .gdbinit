# This is the basis of a debugging session with GDB for a bare metal kernel
# add-auto-load-safe-path /home/ppaolini/kernel/src/kernel/.gdbinit

# Useful defaults for kernel work
#set disassemble-next-line on
#set confirm off
#set pagination off
#set print asm-demangle on
set disassembly-flavor intel

add-symbol-file ./build/kernelfull.o 0x100000
add-symbol-file ./user_space/programs/mainc/build/mainc.elf 0x400000
break idt.c:43
break task.c:133
display/x task_current().registers.ip
display task_current().process.argument
target remote | qemu-system-i386 -hda ./bin/os.bin -S -gdb stdio --accel tcg
layout split
