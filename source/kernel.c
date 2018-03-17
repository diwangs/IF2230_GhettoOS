/* int main() {
     putInMemory(0xB000, 0x8000, 'K');
     putInMemory(0xB000, 0x8001, 0xD);
     putInMemory(0xB000, 0x8002, 'e');
     putInMemory(0xB000, 0x8003, 0xD);
     putInMemory(0xB000, 0x8004, 'r');
     putInMemory(0xB000, 0x8005, 0xD);
     putInMemory(0xB000, 0x8006, 'n');
     putInMemory(0xB000, 0x8007, 0xD);
     putInMemory(0xB000, 0x8008, 'e');
     putInMemory(0xB000, 0x8009, 0XD);
     putInMemory(0xB000, 0x800A, 'l');
     putInMemory(0xB000, 0x800B, 0xD);
     putInMemory(0xB000, 0x800C, '!');
     putInMemory(0xB000, 0x800D, 0xD);
     while (1);
 }

// void handleInterrupt21 (int AX, int BX, int CX, int DX) {}*/

#define MAX_BYTE 256
#define SECTOR_SIZE 512
#define MAX_FILES 16
#define MAX_FILENAME 12
#define MAX_SECTORS 20
#define DIR_ENTRY_LENGTH 32
#define MAP_SECTOR 1
#define DIR_SECTOR 2
#define TRUE 1
#define FALSE 0
#define INSUFFICIENT_SECTORS 0
#define NOT_FOUND -1
#define INSUFFICIENT_DIR_ENTRIES -1
#define EMPTY 0x00
#define USED 0xFF

void handleInterrupt21(int AX, int BX, int CX, int DX);
void printString(char *string);
void readString(char *string);
int mod(int a, int b);
int div(int a, int b);
void readSector(char *buffer, int sector);
void writeSector(char *buffer, int sector);
void readFile(char *buffer, char *filename, int *success);
void clear(char *buffer, int length);
void writeFile(char *buffer, char *filename, int *sectors);
void executeProgram(char *filename, int segment, int *success);

int main()
{
	char x[3];
    makeInterrupt21();
	readString(x);
	printString(x);
	while (1);
}

void handleInterrupt21(int AX, int BX, int CX, int DX) {
    switch (AX) {
    	case 0x0:
    	    printString(BX);
    	    break;
    	case 0x1:
    	    readString(BX);
    	    break;
    	case 0x2:
    	    readSector(BX, CX);
    	    break;
    	case 0x3:
    	    writeSector(BX, CX);
    	    break;
    	case 0x4:
    	    readFile(BX, CX, DX);
    	    break;
    	case 0x5:
    	    writeFile(BX, CX, DX);
    	    break;
    	case 0x6:
    	    executeProgram(BX, CX, DX);
    	    break;
    	default:
    	    printString("Invalid interrupt");
    }
}

void printString(char *string) {
    // masih belum yakin
    int i = 0;
    while (string[i] != '\0') {
        interrupt(0x10, 0xE00 + string[i], 0, 0, 0);
        i++;
    }
}

void readString(char *string) {
	string[0] = interrupt(0x16, 0, 0, 0, 0);
	string[1] = '\0';

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

void writeSector(char *buffer, int sector) {
    interrupt(0x13, 0x301, buffer, div(sector, 36) * 0x100 + mod(sector, 18) + 1, mod(div(sector, 18), 2) * 0x100);
}

void readFile(char *buffer, char *filename, int *success) { /* buat sendiri */ }

void clear(char *buffer, int length) {
    int i;
    for (i = 0; i < length; ++i) buffer[i] = EMPTY;
}

void writeFile(char *buffer, char *filename, int *sectors) {
    char map[SECTOR_SIZE];
    char dir[SECTOR_SIZE];
    char sectorBuffer[SECTOR_SIZE];
    int dirIndex;
    readSector(map, MAP_SECTOR);
    readSector(dir, DIR_SECTOR);
    for (dirIndex = 0; dirIndex < MAX_FILES; ++dirIndex) {
        if (dir[dirIndex * DIR_ENTRY_LENGTH] == '\0') break;
    }
    if (dirIndex < MAX_FILES) {
        int i, j, sectorCount;
        for (i = 0, sectorCount = 0; i < MAX_BYTE && sectorCount < *sectors; ++i) {
            if (map[i] == EMPTY) {
                ++sectorCount;
            }
        }
        if (sectorCount < *sectors) {
            *sectors = INSUFFICIENT_SECTORS;
            return;
        }
        else {
            clear(dir + dirIndex * DIR_ENTRY_LENGTH, DIR_ENTRY_LENGTH);
            for (i = 0; i < MAX_FILENAME; ++i) {
                if (filename[i] != '\0') {
                    dir[dirIndex * DIR_ENTRY_LENGTH + i] = filename[i];
                } else break;
            }
            for (i = 0, sectorCount = 0; i < MAX_BYTE && sectorCount < *sectors; ++i) {
                if (map[i] == EMPTY) {
                    map[i] = USED;
                    dir[dirIndex * DIR_ENTRY_LENGTH + MAX_FILENAME +
                        sectorCount] = i;
                    clear(sectorBuffer, SECTOR_SIZE);
                    for (j = 0; j < SECTOR_SIZE; ++j) {
                        sectorBuffer[j] = buffer[sectorCount * SECTOR_SIZE + j];
                    }
                    writeSector(sectorBuffer, i);
                    ++sectorCount;
                }
            }
        }
    }
    else {
        *sectors = INSUFFICIENT_DIR_ENTRIES;
        return;
    }
    writeSector(map, MAP_SECTOR);
    writeSector(dir, DIR_SECTOR);
}

void executeProgram(char *filename, int segment, int *success) { /* buat sendiri */ }