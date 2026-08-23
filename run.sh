cmake -B build && cmake --build build

./build/Compiler $1

as output.s -o output.o
as runtime/runtime.s -o runtime/runtime_asm.o
gcc -c -O2 -masm=intel -ffreestanding -nostdlib -fno-stack-protector -fno-pic -fno-pie runtime/runtime.c -o runtime/runtime_c.o

ld output.o runtime/runtime_asm.o runtime/runtime_c.o -o program
./program
