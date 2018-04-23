#include "../../library/declaration/constants.h"
#include "../../library/declaration/strutils_dec.h"
#include "../../library/declaration/fsutils_dec.h"

int main() {
    char workingdir;            // current directory; default root
    char prefix[3];
    char input[100];
    char argc;              // jumlah argumen
    char** argv;
    char run_as_background;
    int i;              // indeks string input
    int result;
    char dirs[SECTOR_SIZE];
    enableInterrupts();
    strcpy("$ ", prefix);
    workingdir = 0xFF;

    // Main loop
    while(TRUE) {
        // Init
        i = 0;
        argc = 0;
        run_as_background = 0;
        result = 0;

        // Input 
        do {
            interrupt(0x21, 0x00, prefix, 0, 0);
            interrupt(0x21, 0x01, input, 1, 0); 
        } while(strcmp(input, ""));

        // Chop the spaces in the input, if any
        while (input[i] != '\0') {
            if (input[i] == 0x20) { // 0x20 == space
                argv[argc++] = input + i + 1;
                input[i] = '\0';
            }
            ++i;
        }

        // Detect whether to run the program in the background or not
        if (strcmp(argv[argc - 1], "&")) {
            --argc;
            run_as_background = 1;
        }

        // Execute the input
        if (strcmp(input, "cd")) {      // pindah ke folder
            if (argc == 0) workingdir = 0xFF; else {
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
            if (argc == 0) interrupt(0x21, 0x00, "Usage: pause <pid>\r\n", 0, 0); else {
                if (strcmp(argv[0], "0")) interrupt(0x21, 0x00, "You can't pause the shell\r\n", 0, 0); else {
                    interrupt(0x21, 0x32, ((*argv[0] - '0') + 2) * 4096, &result, 0); 
                    if (result == NOT_FOUND) interrupt(0x21, 0x00, "No running process with such PID\r\n", 0, 0);
                }
            }
        } else if (strcmp(input, "resume")) {
            if (argc == 0) interrupt(0x21, 0x00, "Usage: resume <pid>\r\n", 0, 0); else {
                interrupt(0x21, 0x33, ((*argv[0] - '0') + 2) * 4096, &result, 0); 
                if (result == NOT_FOUND) interrupt(0x21, 0x00, "No paused process with such PID\r\n", 0, 0); 
                else interrupt(0x21, 0x31, 0, 0, 0); // pause the shell
            }
        } else if (strcmp(input, "kill")) {
            if (argc == 0) interrupt(0x21, 0x00, "Usage: kill <pid>\r\n", 0, 0); else {
                if (strcmp(argv[0], "0")) interrupt(0x21, 0x00, "You can't kill the shell\r\n", 0, 0); else {                
                    interrupt(0x21, 0x34, ((*argv[0] - '0') + 2) * 4096, &result, 0); 
                    if (result == NOT_FOUND) interrupt(0x21, 0x00, "No running process with such PID\r\n", 0, 0);
                }
            }
        } else {
            interrupt(0x21, 0x20, workingdir, argc, argv);                // taruh argumen
            interrupt(0x21, (workingdir << 8) | 0x06, input, run_as_background, &result);     // executeProgram
            if (result == NOT_FOUND) interrupt(0x21, 0x00, "No such program\r\n", 0, 0);
            else if (result == INSUFFICIENT_MEMORY) interrupt(0x21, 0x00, "Insufficient memory\r\n", 0, 0);
        }
    }
    return 0;
}

#include "../../library/strutils.h"
#include "../../library/fsutils.h"
