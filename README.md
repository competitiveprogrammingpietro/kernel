# kernel. To be executed with:
qemu-system-x86_64 -hda boot.bin

# Debug
gdb target remote | qemu-system-x86_64 -hda boot.bin -S -gdb stdio

# Disassemble
ndisasm program
bless ./program

# Compile the compiler

Following the instruction at:

https://wiki.osdev.org/GCC_Cross-Compiler

Binutils version used 2.35
GCC verion used: 10.2.0



`
binutils
cd $HOME/src

mkdir build-binutils
cd build-binutils
../binutils-x.y.z/configure --target=$TARGET --prefix="$PREFIX" --with-sysroot --disable-nls --disable-werror
make
make install
This compiles the binutils (assembler, disassembler, and various other useful stuff), runnable on your system but handling code in the format specified by $TARGET.

--disable-nls tells binutils not to include native language support. This is basically optional, but reduces dependencies and compile time. It will also result in English-language diagnostics, which the people on the Forum understand when you ask your questions. ;-)

--with-sysroot tells binutils to enable sysroot support in the cross-compiler by pointing it to a default empty directory. By default, the linker refuses to use sysroots for no good technical reason, while gcc is able to handle both cases at runtime. This will be useful later on.

GDB
It may be worth noting that if you wish to use GDB, and you are running on a different computer architecture than your OS (most common case is developing for ARM on x86_64 or x86_64 on ARM), you need to cross-compile GDB separately. While technically a part of Binutils, it resides in a separate repository.

The protocol for building GDB to target a different architecture is very similar to that of regular Binutils:

../gdb.x.y.z/configure --target=$TARGET --prefix="$PREFIX" --disable-werror
make all-gdb
make install-gdb
The --disable-nls and --with-sysroot options don't seem to have any effect.

GCC
See also the offical instructions for configuring gcc.
Now, you can build GCC.

It will take a while to build your cross-compiler. On a multi-core machine, speed up the build by parallelizing it, e.g. make -j 8 all-gcc, if 8 is the number of jobs to run simultaneously.

If you are building a cross compiler for x86-64, you may want to consider building Libgcc without the "red zone": Libgcc_without_red_zone

cd $HOME/src

# The $PREFIX/bin dir _must_ be in the PATH. We did that above.
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

