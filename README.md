# Course and personal perspective
This repository is the container of the homework/implemenation for the course

https://dragonzap.com/kernel

The course is not very academic and many core concepts, such as pagination, FAT, bootloaders and others required myself to go back to university material, where all of the above was covered but never actually touched first hand.

It might look like a paradox however although quite tiring the *lack* of structure worked to my advantage as it required a good level of self studying back to hardware fundamentals and a decent deal of rewrite, although the main structure greatly resembles the course's one.

The point of the exercise is **not** to present the best kernel in the world, the best algorithms and the best data structures are to be found elsewhere however the challenge is to make a unit of work, a program, that implements a *rudimentary* small OS.

A good deal of the first part of the course has been completed which is why I am convinced that I can present this as part of a job application. Currently the projet implements a single threaded kernel but it is going to be expanded to be a multithreaded kernel as I go along.

Main accomplishement at the moment of writing:

- Bootloader Fat16
- Kernel heap
- Pagination enabled, although flat, not handling of misses yet
- Interrupt handlers PICs structure in place
- Process loading and execution, raw binary and ELF format

truckloads of comments!

# Execution
The build.sh script is going to create a bin/os.bin that can be run with qemu

qemu-system-x86_64 -hda boot.bin

# Debug
gdb target remote | qemu-system-x86_64 -hda boot.bin -S -gdb stdio

# Disassemble
ndisasm program
bless ./program

# Preparation instructions.

This is section is more for me to keep track on what I have done to get the env up and running.

Following the instruction at:

https://wiki.osdev.org/GCC_Cross-Compiler

Binutils version used 2.35
GCC verion used: 10.2.0



`binutils
cd $HOME/src

mkdir build-binutils
cd build-binutils
../binutils-x.y.z/configure --target=$TARGET --prefix="$PREFIX" --with-sysroot --disable-nls --disable-werror
make
make install
This compiles the binutils (assembler, disassembler, and various other useful stuff), runnable on your system but handling code in the format specified by $TARGET.`


The protocol for building GDB to target a different architecture is very similar to that of regular Binutils:

`../gdb.x.y.z/configure --target=$TARGET --prefix="$PREFIX" --disable-werror
make all-gdb
make install-gdb`


GCC
See also the offical instructions for configuring gcc.
Now, you can build GCC.

It will take a while to build your cross-compiler. On a multi-core machine, speed up the build by parallelizing it, e.g. make -j 8 all-gcc, if 8 is the number of jobs to run simultaneously.

If you are building a cross compiler for x86-64, you may want to consider building Libgcc without the "red zone": Libgcc_without_red_zone

`cd $HOME/src

\# The $PREFIX/bin dir _must_ be in the PATH. We did that above.
which -- $TARGET-as || echo $TARGET-as is not in the PATH

mkdir build-gcc
cd build-gcc
../gcc-x.y.z/configure --target=$TARGET --prefix="$PREFIX" --disable-nls --enable-languages=c,c++ --without-headers --disable-hosted-libstdcxx
make all-gcc
make all-target-libgcc
make all-target-libstdc++-v3
make install-gcc
make install-target-libgcc
make install-target-libstdc++-v3
`

