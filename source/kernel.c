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
#define EMPTY 0X00
#define USED 0XFF

void handleInterrupt21 (int AX, int BX, int CX, int DX);
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

int main() {
	makeInterrupt21();
	printString("Hi");
	while (1);
}

void handleInterrupt21 (int AX, int BX, int CX, int DX){
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
	int i=0;	
	while(string[i] != '\0'){
		interrupt(0x10,0xE00+i,0,0,0);	
		i++;
	}		
}

void readString(char *string) {
	char c = interrupt(0x16,0,0,0,0);
}

void executeProgram(char *filename, int segment, int *success) {

}

void readFile(char *buffer, char *filename, int *success){

}

void writeSector(char *buffer, int sector){

}

void writeFile(char *buffer, char *filename, int *sectors){

}

void readSector(char *buffer, int sector){

}
