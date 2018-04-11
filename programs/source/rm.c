// Utility constants
#define TRUE 1
#define FALSE 0
#define INSUFFICIENT_SECTORS 0
#define NOT_FOUND -1
#define INSUFFICIENT_DIR_ENTRIES -1
#define EMPTY 0x00
#define USED 0xFF
// File System Constants
#define MAX_BYTE 256
#define SECTOR_SIZE 512
#define MAX_FILES 16
#define MAX_FILENAME 15
#define MAX_SECTORS 20
#define MAX_ENTRIES 32
#define MAX_DIRS 32
#define ENTRY_LENGTH 16
#define NAME_OFFSET 1
#define DIRS_ENTRY_LENGTH 16
#define FILES_ENTRY_LENGTH 16
#define SECTORS_ENTRY_LENGTH 16
#define MAP_SECTOR 256
#define DIRS_SECTOR 257
#define FILES_SECTOR 258
#define SECTORS_SECTOR 259
#define ARGS_SECTOR 512

void main() {
    int i;
    char curdir;
    char argc;
    char argv[4][32];
    int succ;
    char buff[MAX_FILENAME + 1];
    char dest;
    interrupt(0x21, 0x21, &curdir, 0, 0);
    interrupt(0x21, 0x22, &argc, 0, 0);
    for (i = 0; i < argc; i++) {
        interrupt(0x21, 0x23, i, argv[i], 0);
    }
    if (argc > 0) {
        interrupt(0x21, (curdir << 8) | 0x09, argv[0], &succ, 0);
        if (succ != 0) {
            interrupt(0x21, (curdir << 8) | 0x0A, argv[0], &succ, 0);
        }
        if (succ == -1) {
            interrupt(0x21, 0x0, "File not found.\n", 0, 0);      
        } else if (succ != 0) {
            interrupt(0x21, 0x0, "Unable to remove.\n", 0, 0);      
        }
    }
    interrupt(0x21, (0x00 << 8) | 0x07, &succ, 0, 0);
}