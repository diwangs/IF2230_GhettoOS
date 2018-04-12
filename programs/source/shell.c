/*** the kernel's shell ***/
//#include "../../library/library.h"
// Utility constants
/*#define TRUE 1
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
#define ENTRY_LENGTH 16
#define NAME_OFFSET 1
#define DIRS_ENTRY_LENGTH 16
#define FILES_ENTRY_LENGTH 16
#define SECTORS_ENTRY_LENGTH 16
#define MAP_SECTOR 256
#define DIRS_SECTOR 257
#define FILES_SECTOR 258
#define SECTORS_SECTOR 259
#define ARGS_SECTOR 512*/
#include "../../library/constants.h"

//void readSector(char *buffer, int sector);
char searchDir(char* path, char parentIndex);
//char searchParent(char currentDirectory);
//void printCurDir(char currentDirectory);

int main() {
    char workingdir = 0xFF;            // current directory; default root
    char curdir = workingdir;          // directory yang dipakai untuk execute program
    char prefix[3];
    // char temp[10];
    char input[100];
    // char command[50];       // perintah
    char argc;              // jumlah argumen
    //char argv[20][16];      // isi argumen (max 20)
    char** argv;
    int i;              // indeks string input
    // int j;
    // int argcount;   // jumlah argument count
    int result;
    char dirs[SECTOR_SIZE];

    prefix[0] = '$';
    prefix[1] = ' ';   
    prefix[2] = '\0'; 
    while(1) {
        i = 0;
        argc = 0;
        argv[0] = '\0';
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
        
        /*** memisahkan perintah dan argumen ***/
       
        /*if (input[i] != '\n') {     // ada perintah
            // memperoleh curdir
            if (input[i] == '.') {      // jika di current directory
                i += 2;
                curdir = workingdir;
            } else {                    // jika di root
                curdir = 0xFF;
            }

            while (input[i] != ' ' && input[i] != '\n') {    // sudah selesai membaca perintah
                command[j] = input[i];
                j++;
                i++;
            }
            command[j] = '\0';

            while (input[i] != '\n') {      // membaca argumen jika ada
                i++;            // mengabaikan space
                j = 0;
                while (input[i] != ' ' && input[i] != '\n') {
                    argv[argcount][j] = input[i];
                    j++;
                    i++;
                }
                argv[argcount][j] = '\0';
                argcount++;
            }
        }*/

        if (strcmp(input, "cd")) {      // pindah ke folder
            if (argc == 0) {
                interrupt(0x21, 0x00, "Usage: cd relative_path\r\n", 0, 0);
            } else {            // masuk ke sebuah directory
                if(searchDir(argv[0], workingdir) == 0xFE) {
                    interrupt(0x21, 0x00, "No such directory\r\n", 0, 0);
                } else {
                    workingdir = searchDir(argv[0], workingdir);
                }
            }
        } else if (strcmp(input, "pwd")) {
            // printCurDir(workingdir);
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

/*int strcmp(char* s1, char* s2) {
    int i = 0;
    while (!(s1[i] == '\0' && s2[i] == '\0')) {
        if (s1[i] != s2[i]) return 0;
        ++i;
    }
    return 1;
}

int mod(int a, int b) { 
    while (a >= b) a -= b;
    return a;
}

int div(int a, int b) {
    int q = 0;
    while (q * b <= a) q += 1;
    return q - 1;
}*/
#include "../../library/math.h"
#include "../../library/strutils.h"

/*void readSector(char *buffer, int sector) {
    interrupt(0x13, 0x201, buffer, div(sector, 36) * 0x100 + mod(sector, 18) + 1, mod(div(sector, 18), 2) * 0x100);
}*/

char searchDir(char* path, char parentIndex) { // return the index of the last dirs
    char dirs[SECTOR_SIZE], cur_parent;
    int dirs_offset = 0, dirsname_offset = 0, dirsname_offset_chkp = 0, found = 0;
    interrupt(0x21, 0x2, dirs, DIRS_SECTOR); // readSector itu interrupt
    dirsname_offset = 0;
    cur_parent = parentIndex;
    do { 
        found = 0;
        // Search for dirs
        do { 
            // If the parent directory matches current parent...
            if (dirs[dirs_offset * DIRS_ENTRY_LENGTH] == cur_parent) { 
                // Match the directory name
                found = 1;
                for (dirsname_offset = 0; dirsname_offset <= MAX_FILES && path[dirsname_offset_chkp + dirsname_offset] != '/' && path[dirsname_offset_chkp + dirsname_offset] != '\0'; ++dirsname_offset) { 
                    if (dirs[(dirs_offset * DIRS_ENTRY_LENGTH) + dirsname_offset + 1] != path[dirsname_offset_chkp + dirsname_offset]) {                                
                        found = 0;
                        ++dirs_offset;
                        break;
                    } 
                }
            } else ++dirs_offset;
        } while (!found && dirs_offset < MAX_ENTRIES);
        if (!found) return 0xFE; // If there's no such dirs...
        dirsname_offset_chkp += dirsname_offset + 1;
        cur_parent = dirs_offset;
    } while (path[dirsname_offset_chkp - 1] != '\0');
    return cur_parent;
}

/*char searchParent(char currentDirectory) {
    if (currentDirectory == 0xFF) {
        return currentDirectory;
    } else {
        char dirs[SECTOR_SIZE];
        interrupt(0x21, 0x2, dirs, DIRS_SECTOR);
        return dirs[currentDirectory * DIRS_ENTRY_LENGTH];
    }
}*/

/*void printCurDir(char currentDirectory) {
    char name[16];
    char dirs[SECTOR_SIZE];
    int iname = 0;
    int i;

    if (currentDirectory == 0xFF) {
        name[0] = 'r';
        name[1] = 'o';
        name[2] = 'o';
        name[3] = 't';
        name[4] = '\0';
        
    } else {
        readSector(dirs, DIRS_SECTOR);
        i = (currentDirectory * DIRS_ENTRY_LENGTH) + 1;
        while (dirs[i] != '\0') {
            name[iname] = dirs[i];
            ++i;
            ++iname;
        }
        name[iname] = '\0';
    }
    interrupt(0x21, 0x00, name, 0, 0);
    interrupt(0x21, 0x00, "\r\n", 0, 0);    
}*/