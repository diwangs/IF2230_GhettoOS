#include "../../library/declaration/constants.h"

void main() {
    int* result;
    int i, j;
    char curdir;
    char argv[4][16];
    int succ;
    char buff[MAX_FILENAME + 1];
    char dirs[SECTOR_SIZE];
    char files[SECTOR_SIZE];
    	
    interrupt(0x21, 0x21, &curdir, 0, 0);
    interrupt(0x21, 0x02, dirs, DIRS_SECTOR);
    interrupt(0x21, 0x02, files, FILES_SECTOR);
    interrupt(0x21, 0x0, "List of Directories:\r\n", 0, 0);
    for (i = 0; i < MAX_DIRS; i++) {
        if ((dirs[i * ENTRY_LENGTH + 1] != '\0') && (dirs[i * ENTRY_LENGTH] == curdir)) {
            j = 0;
            while ((j < MAX_FILENAME) && (dirs[i * ENTRY_LENGTH + 1 + j] != '\0')) {
                buff[j] = dirs[i * ENTRY_LENGTH + 1 + j];
                j++;
            }
            buff[j] = '\0';
            interrupt(0x21, 0x0, " - ", 0, 0);
            interrupt(0x21, 0x0, buff, 0, 0);
            interrupt(0x21, 0x0, "\r\n", 0, 0);
        }
    }
    interrupt(0x21, 0x0, "List of Files:\r\n", 0, 0);
    for (i = 0; i < MAX_FILES; i++) {
        if ((files[i * ENTRY_LENGTH + 1] != '\0') && (files[i * ENTRY_LENGTH] == curdir)) {
            j = 0;
            while ((j < MAX_FILENAME) && (files[i * ENTRY_LENGTH + 1 + j] != '\0')) {
                buff[j] = files[i * ENTRY_LENGTH + 1 + j];
                j++;
            }
            buff[j] = '\0';
            interrupt(0x21, 0x0, " - ", 0, 0);
            interrupt(0x21, 0x0, buff, 0, 0);
            interrupt(0x21, 0x0, "\r\n", 0, 0);
        }
    }
    interrupt(0x21, 0x07, result, 0, 0);
}