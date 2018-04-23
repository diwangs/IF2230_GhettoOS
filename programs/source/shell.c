#include "../../library/declaration/constants.h"
#include "../../library/declaration/strutils_dec.h"
#include "../../library/declaration/fsutils_dec.h"

int main() {
    char workingdir = 0xFF;            // current directory; default root
    char prefix[3];
    char input[100];
    char argc;              // jumlah argumen
    char** argv;
    char run_as_background;
    int i;              // indeks string input
    int result;
    char dirs[SECTOR_SIZE];
    enableInterrupts();

    prefix[0] = '$';
    prefix[1] = ' ';   
    prefix[2] = '\0'; 
    while(1) {
        i = 0;
        argc = 0;
        run_as_background = 0;
        interrupt(0x21, 0x00, prefix, 0, 0);// printString("$ ");
        interrupt(0x21, 0x01, input, 1, 0); // readString(input);
        // Chop the spaces, if any
        while (input[i] != '\0') {
            if (input[i] == 0x20) { // 0x20 == space
                argv[argc++] = input + i + 1;
                input[i] = '\0';
            }
            ++i;
        }
        // Detect &
        if (strcmp(argv[argc - 1], "&")) {
            --argc;
            run_as_background = 1;
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
                interrupt(0x21, 0x2, dirs, DIRS_SECTOR, 0); // read sector
                interrupt(0x21, 0x0, dirs + workingdir * DIRS_ENTRY_LENGTH + 1, 0, 0); // print dir name
                interrupt(0x21, 0x00, "\r\n", 0, 0);
            }
        } else if (strcmp(input, "pause")) {
            if (argc == 0) interrupt(0x21, 0x00, "Usage: pause pid\r\n", 0, 0); else {

            }
        } else if (strcmp(input, "resume")) {
            if (argc == 0) interrupt(0x21, 0x00, "Usage: resume pid\r\n", 0, 0); else {
                //printInt((*argv[0] - '0'));
                interrupt(0x21, 0x31, 0, 0, 0); // pause the shell          
                interrupt(0x21, 0x33, ((*argv[0] - '0') + 2) * 4096, &result, 0); // resume the pid
            }
        } else if (strcmp(input, "kill")) {
            if (argc == 0) interrupt(0x21, 0x00, "Usage: kill pid\r\n", 0, 0); else {
                
            }
        } else {
            interrupt(0x21, 0x20, workingdir, argc, argv);                // taruh argumen
            interrupt(0x21, (workingdir << 8) | 0x06, input, run_as_background, &result);     // executeProgram
        }
    }
    return 0;
}

#include "../../library/strutils.h"
#include "../../library/fsutils.h"
