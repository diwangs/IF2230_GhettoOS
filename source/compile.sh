#!/bin/bash

# clean the floppy disk
dd if=/dev/zero of=floppya.img bs=512 count=2880
# assemble the bootloader, put it in the floppy disk
nasm bootload.asm
dd if=bootload of=floppya.img bs=512 count=1 conv=notrunc
# put the file system in the floppy disk
dd if=map.img of=floppya.img bs=512 count=2 seek=1 conv=notrunc
dd if=dir.img of=floppya.img bs=512 count=1 seek=2 conv=notrunc
# compile, link, and put the kernel into the floppy disk
bcc -ansi -c -o kernel.o kernel.c
as86 kernel.asm -o kernel_asm.o
ld86 -o kernel -d kernel.o kernel_asm.o
dd if=kernel of=floppya.img bs=512 conv=notrunc seek=3 

# remove the temporary file
rm kernel.o kernel_asm.o kernel bootload
