/*** the kernel's shell ***/

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

int mod(int a, int b);
int div(int a, int b);
int strcmp(char* s1, char* s2);
void readSector(char *buffer, int sector);
char searchDir(char* path, char parentIndex);
char searchParent(char currentDirectory);


int main() {
    char workingdir = 0xFF;            // current directory; default root
    char curdir = workingdir;          // directory yang dipakai untuk execute program
    char dollar[3];
    char temp[10];
    char input[100];
    char command[50];       // perintah
    char argc;              // jumlah argumen
    char argv[20][16];      // isi argumen (max 20)
    int i = 0;              // indeks string input
    int j = 0;
    int argcount = 0;   // jumlah argument count
    int result;

    dollar[0] = '$';
    dollar[1] = ' ';   
    dollar[2] = '\0'; 
    do {
        interrupt(0x21, 0x00, dollar, 0, 0);
        // printString("$ ");
        
        interrupt(0x21, 0x01, input, 0, 0);
        // readString(input);

        /*** memisahkan perintah dan argumen ***/
       
        if (input[i] != '\n') {     // ada perintah
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
        }

        argc = argcount;     

        if (strcmp(command, "cd")) {      // pindah ke folder
            if (argv[0] == ".." && workingdir != 0xFF) {      // ke parent directory
                workingdir = searchParent(workingdir);
            } else {            // masuk ke sebuah directory
                workingdir = searchDir(argv[0], workingdir);
            }
        } else if (strcmp(command, "pwd")) {

        } else if (!strcmp(command, "exit")) {                    // 
            interrupt(0x21, 0x20, curdir, argc, argv);                          // taruh argumen
            interrupt(0x21, (curdir << 8) | 0x06, command, 0x200, &result);     // executeProgram
        }

        temp[0] = '\n';
        temp[1] = '\0';
        interrupt(0x21, 0x00, temp, 0, 0);
    } while (!strcmp(command, "exit"));
    
    return 0;
}

int mod(int a, int b) { 
    while (a >= b) a -= b;
    return a;
}

int div(int a, int b) {
    int q = 0;
    while (q * b <= a) q += 1;
    return q - 1;
}

void readSector(char *buffer, int sector) {
    interrupt(0x13, 0x201, buffer, div(sector, 36) * 0x100 + mod(sector, 18) + 1, mod(div(sector, 18), 2) * 0x100);
}


int strcmp(char* s1, char* s2) {
    int i = 0;
    while (!(s1[i] == '\0' && s2[i] == '\0')) {
        if (s1[i] != s2[i]) return 0;
        ++i;
    }
    return 1;
}

char searchDir(char* path, char parentIndex) { // return the index of the last dirs
    char dirs[SECTOR_SIZE], cur_parent;
    int dirs_offset = 0, dirsname_offset = 0, dirsname_offset_chkp = 0, found = 0;
    readSector(dirs, DIRS_SECTOR);  
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

char searchParent(char currentDirectory) {
    char dirs[SECTOR_SIZE];
    readSector(dirs, DIRS_SECTOR);
    return dirs[currentDirectory * DIRS_ENTRY_LENGTH];
}