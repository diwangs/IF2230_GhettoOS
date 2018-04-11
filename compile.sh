#!/bin/bash

# Clean the floppy disk
dd if=/dev/zero of=floppya.img bs=512 count=2880 2> /dev/null
echo "Purged floppya.img.."
# Assemble the bootloader, put it in the floppy disk
nasm source/bootload.asm
echo "bootload.asm assembled.."
dd if=source/bootload of=floppya.img bs=512 count=1 conv=notrunc 2> /dev/null
echo "Bootloader loaded.."
# Put the file system in the floppy disk
dd if=source/map.img of=floppya.img bs=512 count=1 seek=256 conv=notrunc 2> /dev/null # previously seek=1
echo "Map sector loaded.."
dd if=source/files.img of=floppya.img bs=512 count=1 seek=258 conv=notrunc 2> /dev/null
echo "File entry sector loaded.."
dd if=source/sectors.img of=floppya.img bs=512 count=1 seek=259 conv=notrunc 2> /dev/null
echo "File sector loaded.."
# Compile, link, and put the kernel into the floppy disk
bcc -ansi -c -o source/kernel.o source/kernel.c
echo "kernel.c compiled.."
as86 source/kernel.asm -o source/kernel_asm.o
echo "kernel.asm assembled.."
ld86 -o source/kernel -d source/kernel.o source/kernel_asm.o
echo "kernel.o and kernel_asm.o linked.."
dd if=source/kernel of=floppya.img bs=512 seek=1 conv=notrunc 2> /dev/null
echo "Kernel loaded.."
# Compile, link, and put the shell
bcc -ansi -c -o programs/shell.o programs/shell.c
echo "shell.c compiled.."
as86 programs/lib.asm -o programs/lib_asm.o
echo "lib.asm assembled.."
ld86 -o programs/shell -d programs/shell.o programs/lib_asm.o
echo "shell.o and lib_asm.o linked.."
bcc -ansi -c -o programs/echo.o programs/echo.c
ld86 -o programs/echo -d programs/echo.o programs/lib_asm.o
# # Remove the temporary file
# rm source/kernel.o source/kernel_asm.o source/kernel source/bootload

# # Remove the temporary file
# rm source/kernel.o source/kernel_asm.o source/kernel source/bootload

# Load the test file
cd programs
gcc loadFile.c -o loadFile -w
# cd ..
./loadFile shell
./loadFile echo

echo "GhettoOS compiled successfully!"
