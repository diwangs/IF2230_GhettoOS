#!/bin/bash

# Clean the floppy disk
dd if=/dev/zero of=floppya.img bs=512 count=2880 2> /dev/null
echo "Purged floppya.img"
# Assemble the bootloader, put it in the floppy disk
nasm source/bootload.asm
echo "bootload.asm assembled"
dd if=source/bootload of=floppya.img bs=512 count=1 conv=notrunc 2> /dev/null
echo "Bootloader loaded"
# Put the file system in the floppy disk
dd if=source/map.img of=floppya.img bs=512 count=1 seek=256 conv=notrunc 2> /dev/null # previously seek=1
echo "Map sector loaded"
dd if=source/files.img of=floppya.img bs=512 count=1 seek=258 conv=notrunc 2> /dev/null
echo "File entry sector loaded"
dd if=source/sectors.img of=floppya.img bs=512 count=1 seek=259 conv=notrunc 2> /dev/null
echo "File sector loaded"
# Compile, link, and put the kernel into the floppy disk
bcc -ansi -c -o source/proc.o source/proc.c
echo "proc.c compiled"
bcc -ansi -c -o source/kernel.o source/kernel.c
echo "kernel.c compiled"
as86 source/kernel.asm -o source/kernel_asm.o
echo "kernel.asm assembled"
ld86 -o source/kernel -d source/kernel.o source/kernel_asm.o source/proc.o
echo "kernel.o, kernel_asm.o, and proc.o linked"
dd if=source/kernel of=floppya.img bs=512 seek=1 conv=notrunc 2> /dev/null
echo "Kernel loaded"
# Remove the temporary file
rm source/kernel.o source/kernel_asm.o source/kernel source/bootload source/proc.o

# Compile the loadFile program
cd programs
gcc source/_loadFile.c -o loadFile -w
echo "loadFile compiled"

# Assemble lib.asm
as86 source/lib.asm -o lib.o
echo "lib.asm assembled"

# Compile, link, and load the shell
bcc -ansi -c -o shell.o source/shell.c
echo "shell.c compiled"
ld86 -o shell -d shell.o lib.o
echo "shell.o and lib.o linked"
./loadFile shell
rm shell.o shell

# Compile, link, and load echo
bcc -ansi -c -o echo.o source/echo.c
echo "echo.c compiled"
ld86 -o echo -d echo.o lib.o
echo "echo.o and lib.o linked"
./loadFile echo
rm echo.o echo

# Compile, link, and load mkdir
bcc -ansi -c -o mkdir.o source/mkdir.c
echo "mkdir.c compiled"
ld86 -o mkdir -d mkdir.o lib.o
echo "mkdir.o and lib.o linked"
./loadFile mkdir
rm mkdir.o mkdir

# Compile, link, and load ls
bcc -ansi -c -o ls.o source/ls.c
echo "ls.c compiled"
ld86 -o ls -d ls.o lib.o
echo "ls.o and lib.o linked"
./loadFile ls
rm ls.o ls

# Compile, link, and load rm
bcc -ansi -c -o rm.o source/rm.c
echo "rm.c compiled"
ld86 -o rm -d rm.o lib.o
echo "rm.o and lib.o linked"
./loadFile rm
rm rm.o rm

# Compile, link, and load rm
bcc -ansi -c -o cat.o source/cat.c
echo "cat.c compiled"
ld86 -o cat -d cat.o lib.o
echo "cat.o and lib.o linked"
./loadFile cat
rm cat.o cat

./loadFile keyproc2

rm lib.o loadFile

echo "GhettoOS compiled successfully!"
