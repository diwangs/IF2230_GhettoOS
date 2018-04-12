#include "../../library/declaration/constants.h"
#include "../../library/declaration/strutils_dec.h"
#include "../../library/declaration/fsutils_dec.h"

int main() {
    char workingdir = 0xFF;            // current directory; default root
    char curdir = workingdir;          // directory yang dipakai untuk execute program
    char prefix[3];
    char input[100];
    char argc;              // jumlah argumen
    char** argv;
    int i;              // indeks string input
    int result;
    char dirs[SECTOR_SIZE];

    prefix[0] = '$';
    prefix[1] = ' ';   
    prefix[2] = '\0'; 
    while(1) {
        i = 0;
        argc = 0;
        // argv[0] = '\0';
        interrupt(0x21, 0x00, prefix, 0, 0);// printString("$ ");
        interrupt(0x21, 0x01, input, 0, 0); // readString(input);
        // Chop the spaces, if any
        while (input[i] != '\0') {
            if (input[i] == 0x20) { // 0x20 == space
                argv[argc++] = input + i + 1;
                input[i] = '\0';
            }
            ++i;
        }

        if (strcmp(input, "cd")) {      // pindah ke folder
            if (argc == 0) {
                interrupt(0x21, 0x00, "Usage: cd relative_path\r\n", 0, 0);
            } else {            // masuk ke sebuah directory
                if(searchPath(argv[0], workingdir) == 0xFE) {
                    interrupt(0x21, 0x00, "No such directory\r\n", 0, 0);
                } else {
                    workingdir = searchPath(argv[0], workingdir);
                }
            }
        } else if (strcmp(input, "pwd")) {
            if (workingdir == 0xFF) {
                interrupt(0x21, 0x00, "root\r\n", 0, 0);
            } else {
                interrupt(0x21, 0x2, dirs, DIRS_SECTOR); // read sector
                interrupt(0x21, 0x0, dirs + workingdir * DIRS_ENTRY_LENGTH + 1, 0, 0); // print dir name
                interrupt(0x21, 0x00, "\r\n", 0, 0);
            }
        } else {
            interrupt(0x21, 0x20, curdir, argc, argv);                          // taruh argumen
            interrupt(0x21, (curdir << 8) | 0x06, input, 0x200, &result);     // executeProgram
        }
    }
    return 0;
}

#include "../../library/strutils.h"
#include "../../library/fsutils.h"
