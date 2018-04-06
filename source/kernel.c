#define MAX_BYTE 256
#define SECTOR_SIZE 512
#define MAX_FILES 16
#define MAX_FILENAME 15
#define MAX_SECTORS 20
#define DIRS_ENTRY_LENGTH 16
#define FILES_ENTRY_LENGTH 16
#define SECTORS_ENTRY_LENGTH 16
#define MAP_SECTOR 256
#define DIRS_SECTOR 257
#define FILES_SECTOR 258
#define SECTORS_SECTOR 259
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
void readFile(char *buffer, char *path, int *result, char parentIndex);
void clear(char *buffer, int length);
void writeFile(char *buffer, char *filename, int *sectors);
void executeProgram(char *filename, int segment, int *success);
void printLogo();

int main() {		
	int *suc;
	makeInterrupt21();	
	//printLogo();
	executeProgram("keyproc", 0x2000, suc);
	while(1) {}
}

/*void handleInterrupt21(int AX, int BX, int CX, int DX) { // asm linking
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
}*/

void handleInterrupt21 (int AX, int BX, int CX, int DX) { // asm linking
	char AL, AH;
	AL = (char) (AX);
	AH = (char) (AX >> 8);

	switch (AL) {
		case 0x00:
			printString(BX);
			break;
		case 0x01:
			readString(BX);
			break;
		case 0x02:
			readSector(BX, CX);
			break;
		case 0x03:
			writeSector(BX, CX);
			break;
		case 0x04:
			readFile(BX, CX, DX, AH);
			break;
		case 0x05:
			writeFile(BX, CX, DX, AH);
			break;
		case 0x06:
			executeProgram(BX, CX, DX, AH);
			break;
		case 0x07:
			terminateProgram(BX);
			break;
		case 0x08:
			makeDirectory(BX, CX, AH);
			break;
		case 0x09:
			deleteFile(BX, CX, AH);
			break;
		case 0x0A:
			deleteDirectory(BX, CX, AH);
			break;
		case 0x20:
			putArgs(BX, CX);
			break;
		case 0x21:
			getCurdir(BX);
			break;
		case 0x22:
			getArgc(BX);
			break;
		case 0X23:
			getArgv(BX, CX);
			break;
		default:
			printString("Invalid interrupt");
	}
}


void printString(char *string) { // Works like println
	int i = 0;
	while (string[i] != '\0') interrupt(0x10, 0xE00 + string[i++], 0, 0, 0);
	interrupt(0x10, 0xE00 + '\r', 0, 0, 0);       
	interrupt(0x10, 0xE00 + '\n', 0, 0, 0);
}

void readString(char *string) {
	int index = 0;
	char input_buffer = 0x00; // To remember last input
	do {
		input_buffer = interrupt(0x16, 0, 0, 0, 0); // Read a character from the keyboard
		interrupt(0x10, 0xE00 + input_buffer, 0, 0, 0); // Print it
		if (input_buffer == '\r') string[index] = '\0'; // If it's an ENTER, terminate it with a NULL
		else if (input_buffer != '\b') string[index++] = input_buffer; // If it's not a backspace, input it
		else if (index-- > 0) { // If it is backspace, space over the last character
			interrupt(0x10, 0xE00 + 0x20, 0, 0, 0);
			interrupt(0x10, 0xE00 + '\b', 0, 0, 0);
		}
	} while (input_buffer != '\r'); // Wait for ENTER 
	interrupt(0x10, 0xE00 + '\n', 0, 0, 0);
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

void readFile(char *buffer, char *path, int *result, char parentIndex) {
	/*char dir[SECTOR_SIZE];
	int found = 0, i = 0, j = 0;		
	readSector(dir, DIR_SECTOR);
	// Search the sector for the file
	while (!found && i * DIR_ENTRY_LENGTH < SECTOR_SIZE) {
		// Match the filename
		found = 1;
		for (j = 0; j < MAX_FILES && filename[j] != '\0'; ++j) {
			if (dir[i * DIR_ENTRY_LENGTH + j] != filename[j]) {
				found = 0;
				break;
			} 
		}
		if(!found) ++i;
	}
	// If the file is not found
	if (!found) {
		*success = 0;
		return;
	}
	// Else, read the file sector
	for (j = 0; j < MAX_SECTORS && dir[i * DIR_ENTRY_LENGTH + MAX_FILENAME + j] != '\0'; ++j) {	
		readSector(buffer + j * SECTOR_SIZE, dir[i * DIR_ENTRY_LENGTH + MAX_FILENAME + j]);
	}	
	*success = 1;
	return;*/

	char dirs[SECTOR_SIZE], files[SECTOR_SIZE], sectors[SECTOR_SIZE];
	int dirs_offset = 0, dirsname_offset = 0, last_slash_idx = 0, dirsname_offset_chkp = 0, cur_parent = 0, found = 0;
	int files_offset = 0, filesname_offset = 0;
	int sectors_offset = 0;
	// Find the index of the last slash, to determine when to search for the filename instead of dirsname
	while (path[dirsname_offset] != '\0') {
		if (path[dirsname_offset] == '/') last_slash_idx = dirsname_offset;
		++dirsname_offset;
	}
	dirsname_offset = 0;
	// Search for path
	readSector(dirs, DIRS_SECTOR);	
	dirsname_offset = 0;
	cur_parent = parentIndex;
	while (dirsname_offset_chkp != last_slash_idx) { 
		found = 0;
		do { // Search for dirs
			if (dirs[dirs_offset * DIRS_ENTRY_LENGTH] == cur_parent) { // If the parent directory matches current parent...
				// Match the directory name
				found = 1;
				for (dirsname_offset = 1; dirsname_offset <= MAX_FILES && path[dirsname_offset_chkp + dirsname_offset] != '/'; ++dirsname_offset) {
					if (dirs[(dirs_offset * DIRS_ENTRY_LENGTH) + dirsname_offset] != path[dirsname_offset_chkp + dirsname_offset]) {
						found = 0;
						++dirs_offset;
						break;
					} 
				}
			}
		} while (!found && dirs_offset < MAX_SECTORS);
		if (!found) { // If there's no such dirs...
			*result = -1;
			return;
		}
		dirsname_offset_chkp += dirsname_offset;
		cur_parent = dirs_offset;
	}
	// Search for the file
	readSector(files, FILES_SECTOR);
	found = 0;
	do { // Search for files
		if (files[files_offset * FILES_ENTRY_LENGTH] == cur_parent) { // If the parent directory matches current parent...
			// Match the file name
			found = 1;
			for (filesname_offset = 1; filesname_offset <= MAX_FILES && path[dirsname_offset_chkp + filesname_offset] != '\0'; ++filesname_offset) {
				if (files[(files_offset * FILES_ENTRY_LENGTH) + filesname_offset] != path[dirsname_offset_chkp + filesname_offset]) {
					found = 0;
					++files_offset;
					break;
				} 
			}
		}
	} while (!found && files_offset < MAX_FILES);
	if (!found) { // If there's no such file...
		*result = -2;
		return;
	}
	// Read the file from its sectors
	readSector(sectors, SECTORS_SECTOR);
	do {
		readSector(buffer + (sectors_offset * SECTOR_SIZE), sectors[(files_offset * SECTORS_ENTRY_LENGTH) + sectors_offset]);
		++sectors_offset;
	} while(sectors_offset != '\0');
	*result = 0;
}

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
	readSector(dir, DIRS_SECTOR);
	for (dirIndex = 0; dirIndex < MAX_FILES; ++dirIndex) {
		if (dir[dirIndex * DIRS_ENTRY_LENGTH] == '\0') break;
	}
	if (dirIndex < MAX_FILES) {
		int i, j, sectorCount;
		for (i = 0, sectorCount = 0; i < MAX_BYTE && sectorCount < *sectors; ++i) {
			if (map[i] == EMPTY) ++sectorCount;
		}
		if (sectorCount < *sectors) {
			*sectors = INSUFFICIENT_SECTORS;
			return;
		} else {
			clear(dir + dirIndex * DIRS_ENTRY_LENGTH, DIRS_ENTRY_LENGTH);
			for (i = 0; i < MAX_FILENAME; ++i) {
				if (filename[i] != '\0') {
					dir[dirIndex * DIRS_ENTRY_LENGTH + i] = filename[i];
				} else break;
			}
			for (i = 0, sectorCount = 0; i < MAX_BYTE && sectorCount < *sectors; ++i) {
				if (map[i] == EMPTY) {
					map[i] = USED;
					dir[dirIndex * DIRS_ENTRY_LENGTH + MAX_FILENAME +
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
	} else {
		*sectors = INSUFFICIENT_DIR_ENTRIES;
		return;
	}
	writeSector(map, MAP_SECTOR);
	writeSector(dir, DIRS_SECTOR);
}

void executeProgram(char *filename, int segment, int *success) {
	char buffer[MAX_SECTORS * SECTOR_SIZE];	
	int i;				
	readFile(buffer, filename, success); 
	if (!*success) return; 
	for (i = 0; i < MAX_SECTORS * SECTOR_SIZE; ++i) putInMemory(segment, i, buffer[i]);			
	launchProgram(segment); 
}

void printCenter(int row, int ln, char* s){
	 int i = 0;
	 int offset = 0x8000 + ((row-1)*80*2) + (40-ln/2)*2;
	 while(s[i] != '\0'){
			putInMemory(0xB000, offset + i*2, s[i]);
			i++;
	 }
}

void printLogo(){
	int i = 0;
	
	// clear the screen
	while (i < 4000){
		putInMemory(0xB000, 0x8000 + i*2, '\0');
		i++;
	}

	// print logo in center
	printCenter(7, 20, "========        ");
	 	printCenter(8, 20, "==               ");
	 	printCenter(9, 20, "==   ===        ");
	 	printCenter(10, 20,"==    ==        ");
	 	printCenter(11, 20,"========        ");
	 	printCenter(12, 20,"  / \\   ==   ==");
	 	printCenter(13, 20," /   \\  ==   ==");
	 	printCenter(14, 20,"/ 0 0 \\ =======");
	 	printCenter(15, 20,"\\  .  / =======");
	 	printCenter(16, 20," \\ v /  ==   ==");
	 	printCenter(17, 20,"  \\ /   ==   ==");
	 	printCenter(18, 20," Ghetto ==   ==");
	printString("\n\n\nPress any key to continue...");
	 	interrupt(0x16,0,0,0,0);

}