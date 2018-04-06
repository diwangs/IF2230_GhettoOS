#!/bin/bash

# Clean the floppy disk
dd if=/dev/zero of=floppya.img bs=512 count=2880
# Assemble the bootloader, put it in the floppy disk
nasm bootload.asm
dd if=bootload of=floppya.img bs=512 count=1 conv=notrunc
# Put the file system in the floppy disk
dd if=map.img of=floppya.img bs=512 count=2 seek=256 conv=notrunc # previously 1
# dd if=dir.img of=floppya.img bs=512 count=1 seek=2 conv=notrunc, 257 for dirs
dd if=files.img of=floppya.img bs=512 count=1 seek=258 conv=notrunc
dd if=sectors.img of=floppya.img bs=512 count=1 seek=259 conv=notrunc
# Compile, link, and put the kernel into the floppy disk
bcc -ansi -c -o kernel.o kernel.c
as86 kernel.asm -o kernel_asm.o
ld86 -o kernel -d kernel.o kernel_asm.o
dd if=kernel of=floppya.img bs=512 conv=notrunc seek=3 
# Load the keyproc program
# gcc loadFile.c -o loadFile
# ./loadFile keyproc

# Remove the temporary file
rm kernel.o kernel_asm.o kernel bootload loadFile
