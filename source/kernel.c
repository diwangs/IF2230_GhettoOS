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

void handleInterrupt21(int AX, int BX, int CX, int DX); // asm linking purposes
// Utility
void printString(char *string);
void readString(char *string);
int mod(int a, int b); // Fucking bcc can't understand / and %
int div(int a, int b);
void clear(char *buffer, int length);
void printLogo();
// File System
void readSector(char *buffer, int sector);
void writeSector(char *buffer, int sector);
void readFile(char *buffer, char *path, int *result, char parentIndex);
void writeFile(char *buffer, char *path, int *sectors, char parentIndex);
void deleteFile(char *path, int *result, char parentIndex); 
void makeDirectory(char *path, int *result, char parentIndex); // Not yet
void deleteDirectory(char *path, int *success, char parentIndex); // Not yet
// Execute a Program
void getCurdir (char *curdir);
void getArgc (char *argc);
void getArgv (char index, char *argv);
void putArgs (char curdir, char argc, char **argv);
void executeProgram(char *path, int segment, int *result, char parentIndex); // segment = 0x2000
void terminateProgram(int* result); 

int main() {		
	int* result;
	char buf[512];
	makeInterrupt21();
	writeFile("hehehoo", "/hehehe", result, 0xFF);
	readFile(buf, "/hehehe", result, 0xFF);
	if (!*result) printString(buf); else printString("Failed");
	deleteFile("/hehehe", result, 0xFF);
	readFile(buf, "/hehehe", result, 0xFF);	
	if (!*result) printString(buf); else printString("Failed");	
	while(1) {}
}

void handleInterrupt21 (int AX, int BX, int CX, int DX) {
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


//=======================================================================================
// Utility
//=======================================================================================

void printString(char *string) { // Works like println
	int i = 0;
	while (string[i] != '\0') interrupt(0x10, 0xE00 + string[i++], 0, 0, 0);
	interrupt(0x10, 0xE00 + '\r', 0, 0, 0);       
	interrupt(0x10, 0xE00 + '\n', 0, 0, 0);
}

void printInt(int i) {
	char ir = '0' + (char) div(i, 100);
	char ip = '0' + (char) div(mod(i, 100), 10);
	char is = '0' + (char) mod(i, 10);
	interrupt(0x10, 0xE00 + ir, 0, 0, 0);
	interrupt(0x10, 0xE00 + ip, 0, 0, 0);
	interrupt(0x10, 0xE00 + is, 0, 0, 0);
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

void clear(char *buffer, int length) {
	int i;
	for (i = 0; i < length; ++i) buffer[i] = EMPTY;
}

int findUnusedSector (char *map) {
  int i;
  for (i = 0; i < MAX_BYTE; ++i) {
    if (map[i] == 0x00) {
      return i;
    }
  }
  return NOT_FOUND;
}

int findUnusedEntry (char *entries) {
	int i;
	for (i = 0; i < MAX_ENTRIES; ++i) {
		if (entries[i * ENTRY_LENGTH + NAME_OFFSET] == '\0') {
			return i;
		}
	}
	return NOT_FOUND;
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
	printCenter(8, 20, "==              ");
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


//=======================================================================================
// File System
//=======================================================================================

void readSector(char *buffer, int sector) {
	interrupt(0x13, 0x201, buffer, div(sector, 36) * 0x100 + mod(sector, 18) + 1, mod(div(sector, 18), 2) * 0x100);
}

void writeSector(char *buffer, int sector) {
	interrupt(0x13, 0x301, buffer, div(sector, 36) * 0x100 + mod(sector, 18) + 1, mod(div(sector, 18), 2) * 0x100);
}

void readFile(char *buffer, char *path, int *result, char parentIndex) {
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
		} else ++files_offset;
	} while (!found && files_offset < MAX_FILES);
	if (!found) { // If there's no such file...
		*result = -2;
		return;
	}
	// Read the file from its sectors
	readSector(sectors, SECTORS_SECTOR);
	while(sectors[files_offset * SECTORS_ENTRY_LENGTH + sectors_offset] != '\0' && sectors_offset < SECTORS_ENTRY_LENGTH) {
		readSector(buffer + sectors_offset * SECTOR_SIZE, sectors[files_offset * SECTORS_ENTRY_LENGTH + sectors_offset]);
		++sectors_offset;
	} 
	*result = 0;
}

void writeFile(char *buffer, char *path, int *sectors, char parentIndex) {
	char map[SECTOR_SIZE], dirs[SECTOR_SIZE], files[SECTOR_SIZE], sectors[SECTOR_SIZE];
	int map_offset = 0;
	int dirs_offset = 0, dirsname_offset = 0, dirsname_offset_chkp = 0, cur_parent = 0, found = 0;
	int files_offset = 0, filesname_offset = 0, empty_files_index = 0;
	int sectors_offset = 0, empty_sector = 0;
	int last_slash_idx;
	// Search for empty sector
	readSector(map, MAP_SECTOR);
	map_offset = findUnusedSector(map);
	// If there's no empty sectors
	if (map_offset == NOT_FOUND) {
		*sectors = 0x00;
		return;
	}
	// Search for empty file entry
	readSector(files, FILES_SECTOR);
	for (empty_files_index = 0; files_offset < MAX_FILES; ++files_offset) {
		if (files[files_offset * DIRS_ENTRY_LENGTH + 1] == '\0') break;
	}
	// If there's no empty entries
	if (files_offset == MAX_FILES) {
		*sectors = -3;
		return;
	}
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
			*sectors = -1;
			return;
		}
		dirsname_offset_chkp += dirsname_offset;
		cur_parent = dirs_offset;
	}
	// Search whether the file exists or not
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
		} else ++files_offset;
	} while (!found && files_offset < MAX_FILES);
	if (found) { // If there's such file...
		*sectors = -2;
		return;
	}
	// Write to file entry
	files[empty_files_index] = (char) cur_parent;
	dirsname_offset = 1;
	while (path[last_slash_idx + dirsname_offset] != '\0') {
		files[empty_files_index + dirsname_offset] = path[last_slash_idx + dirsname_offset];	
		++dirsname_offset;
	}
	// Write buffer
	while (buffer[sectors_offset * SECTOR_SIZE] != '\0') {
		empty_sector = findUnusedSector(map);
		writeSector(buffer + sectors_offset * SECTOR_SIZE, empty_sector);
		sectors[empty_files_index + sectors_offset] = empty_sector;
		map[empty_sector] = 0xFF;
		++sectors_offset;
	}
	// Write to floppy
	writeSector(map, MAP_SECTOR);
	writeSector(dirs, DIRS_SECTOR);
	writeSector(files, FILES_SECTOR);
	writeSector(sectors, SECTORS_SECTOR);
}

void deleteFile(char *path, int *result, char parentIndex) {
	char map[SECTOR_SIZE], dirs[SECTOR_SIZE], files[SECTOR_SIZE], sectors[SECTOR_SIZE];
	int dirs_offset = 0, dirsname_offset = 0, last_slash_idx = 0, dirsname_offset_chkp = 0, cur_parent = 0, found = 0;
	int files_offset = 0, filesname_offset = 0;
	int sectors_offset = 0;
	
	// Find the index of the last slash, to determine when to search for the filename instead of dirsname
	while (path[dirsname_offset] != '\0') {	
		if (path[dirsname_offset] == '/') last_slash_idx = dirsname_offset;
		++dirsname_offset;
	}
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
		} else ++files_offset;
	} while (!found && files_offset < MAX_FILES);
	if (!found) { // If there's no such file...
		*result = -2;
		return;
	}
	// Delete in map
	readSector(map, MAP_SECTOR);
	readSector(sectors, SECTORS_SECTOR);
	sectors_offset = 0;
	while (sectors[files_offset * SECTORS_ENTRY_LENGTH + sectors_offset] != '\0') {
		map[sectors[files_offset * SECTORS_ENTRY_LENGTH + sectors_offset]] = 0x00;
		++sectors_offset;
	}
	// Delete file entry
	files[files_offset * FILES_ENTRY_LENGTH + 1] = '\0';
	*result = 0;
	// Write buffer
	writeSector(map, MAP_SECTOR);
	writeSector(dirs, DIRS_SECTOR);
	writeSector(files, FILES_SECTOR);
	writeSector(sectors, SECTORS_SECTOR);
}


void makeDirectory(char *path, int *result, char parentIndex) {
	char dirs[SECTOR_SIZE], sectors[SECTOR_SIZE];
	int dirLine, found, i;
	int dirsOffset = 0, dirsNameOffset = 0, lastSlashIdx = 0,
	dirsNameOffsetChkp = 0, curParent = 0, sectorsOffset = 0;
	int unusedEntry;
	readSector(dirs,DIRS_SECTOR);

	// Check empty entries in dir
	unusedEntry = findUnusedEntry(dirs);

	if (unusedEntry == NOT_FOUND){ // No empty location
		*result = -3;
		return;
	}

	// Find the index of last slash ,to gain directory name that want to be created
	while(path[dirsNameOffset] != '\0') {
		if(path[dirsNameOffset] == '/') {
			lastSlashIdx = dirsNameOffset;
		}
		dirsNameOffset += 1;
	}

	dirsNameOffset = 0;
	curParent = parentIndex;

	while(dirsNameOffsetChkp != lastSlashIdx) {
		found = 0;
		do {
			// If the parent directory matches current parent
			if(dirs[dirsOffset * DIRS_ENTRY_LENGTH == curParent]) {
				found = 1;
				for(dirsNameOffset = 1; dirsNameOffset <= MAX_FILES && path[dirsNameOffsetChkp + dirsNameOffset] != '/'; ++dirsNameOffset) {
					if (dirs[(dirsOffset * DIRS_ENTRY_LENGTH) + dirsNameOffset] != path[dirsNameOffsetChkp + dirsNameOffset]) {
						found = 0;
						++dirsOffset;
						break;
					} 
				}
			}
		} while (!found && dirsOffset < MAX_SECTORS);
		if (!found) { // No such dirs
			*result = -1;
			return;
		}
		// If dirs is avail, search next dir
		dirsNameOffsetChkp += dirsNameOffset;
		curParent = dirsOffset;
	}

	found = 0;
	do {
		if(dirs[dirsOffset * DIRS_ENTRY_LENGTH] == curParent) {
			found = 1;	
			for(dirsNameOffset = 1; dirsNameOffset <= MAX_FILES && path[dirsNameOffsetChkp + dirsNameOffset] != '\0'; ++dirsNameOffset) {
				if (dirs[(dirsOffset * DIRS_ENTRY_LENGTH) + dirsNameOffset] != path[dirsNameOffsetChkp + dirsNameOffset]) {
					found = 0;
					++dirsOffset;
					break;
				} 
			}
		}
	} while(!found && dirsOffset < MAX_SECTORS);
	
	// No dirs yet, write directory
	if(!found) {
		dirs[unusedEntry] = curParent;
		i = 1;
		for(dirsNameOffset = lastSlashIdx + 1; path[dirsNameOffset] != '\0'; ++dirsNameOffset) {
			dirs[unusedEntry + i] = path[dirsNameOffset];
			i++;
		}
	} else { // Dirs already exist
		*result = -2;
		return;
	}
	writeSector(dirs,DIRS_SECTOR);

}

void deleteDirectory(char *path, int *success, char parentIndex) {
	char dirs[SECTOR_SIZE], sectors[SECTOR_SIZE], map[SECTOR_SIZE];
	int dirLine, found;
	int dirsOffset = 0, dirsNameOffset = 0, lastSlashIdx = 0,
	dirsNameOffsetChkp = 0, curParent = 0, sectorsOffset = 0;
	readSector(dirs,DIRS_SECTOR);
	readSector(map,MAP_SECTOR);

	// Find the index of last slash ,to gain directory name that want to be created
	while(path[dirsNameOffset] != '\0') {
		if(path[dirsNameOffset] == '/') {
			lastSlashIdx = dirsNameOffset;
		}
		dirsNameOffset += 1;
	}

	dirsNameOffset = 0;
	curParent = parentIndex;

	while(dirsNameOffsetChkp != lastSlashIdx) {
		found = 0;
		do {
			// If the parent directory matches current parent
			if(dirs[dirsOffset * DIRS_ENTRY_LENGTH == curParent]) {
				found = 1;
				for(dirsNameOffset = 1; dirsNameOffset <= MAX_FILES && path[dirsNameOffsetChkp + dirsNameOffset] != '/'; ++dirsNameOffset) {
					if (dirs[(dirsOffset * DIRS_ENTRY_LENGTH) + dirsNameOffset] != path[dirsNameOffsetChkp + dirsNameOffset]) {
						found = 0;
						++dirsOffset;
						break;
					} 
				}
			}
		} while (!found && dirsOffset < MAX_SECTORS);
		if (!found) { // No such dirs
			*success = -1;
			return;
		}
		// If dirs is avail, search next dir
		dirsNameOffsetChkp += dirsNameOffset;
		curParent = dirsOffset;
	}

	// Search for dir that want to be deleted
	found = 0;
	do {
		if(dirs[dirsOffset * DIRS_ENTRY_LENGTH] == curParent) {
			found = 1;	
			for(dirsNameOffset = 1; dirsNameOffset <= MAX_FILES && path[dirsNameOffsetChkp + dirsNameOffset] != '\0'; ++dirsNameOffset) {
				if (dirs[(dirsOffset * DIRS_ENTRY_LENGTH) + dirsNameOffset] != path[dirsNameOffsetChkp + dirsNameOffset]) {
					found = 0;
					++dirsOffset;
					break;
				} 
			}
		}
	} while(!found && dirsOffset < MAX_SECTORS);

	// Dir not found
	if(!found) {
		*success = -1;
		return;
	}

	// Back to current parent
	dirsOffset = curParent;
}


//=======================================================================================
// Program execution
//=======================================================================================

void putArgs (char curdir, char argc, char **argv) {
	char args[SECTOR_SIZE];
	int i, j, p;
	clear(args, SECTOR_SIZE);
	args[0] = curdir;
	args[1] = argc;
	i = 0;
	j = 0;
	for (p = 1; p < ARGS_SECTOR && i < argc; ++p) {
		args[p] = argv[i][j];
		if (argv[i][j] == '\0') {
			++i;
			j = 0;
		} else ++j;
	}
	writeSector(args, ARGS_SECTOR);
}

void getCurdir (char *curdir) {
	char args[SECTOR_SIZE];
	readSector(args, ARGS_SECTOR);
	*curdir = args[0];
}

void getArgc (char *argc) {
	char args[SECTOR_SIZE];
	readSector(args, ARGS_SECTOR);
	*argc = args[1];
}

void getArgv (char index, char *argv) {
	char args[SECTOR_SIZE];
	int i, j, p;
	readSector(args, ARGS_SECTOR);
	i = 0;
	j = 0;
	for (p = 1; p < ARGS_SECTOR; ++p) {
		if (i == index) {
			argv[j] = args[p];
			++j;
		}
		if (args[p] == '\0') {
			if (i == index) break; else ++i;
		}
	}
}

void executeProgram(char *path, int segment, int *result, char parentIndex) {
	char buffer[MAX_SECTORS * SECTOR_SIZE];	
	int i;				
	readFile(buffer, path, result, parentIndex); 
	if (*result) return; 
	for (i = 0; i < MAX_SECTORS * SECTOR_SIZE; ++i) putInMemory(segment, i, buffer[i]);			
	launchProgram(segment); 
}

void terminateProgram (int *result) {
	char shell[7];
	shell[0] = '/';
	shell[1] = 's';
	shell[2] = 'h';
	shell[3] = 'e';
	shell[4] = 'l';
	shell[5] = 'l';
	shell[6] = '\0';
	executeProgram(shell, 0x2000, result, 0xFF);
}