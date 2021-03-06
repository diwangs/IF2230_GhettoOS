#include "../library/declaration/constants.h"
#define MAIN
#include "proc.h"

void handleInterrupt21(int AX, int BX, int CX, int DX); // asm linking purposes
// Utility
void printString(char *string);
void readString(char *string, int disableProcessControl);
#include "../library/declaration/math_dec.h"
#include "../library/declaration/strutils_dec.h"
#include "../library/declaration/fsutils_dec.h"
void printLogo();
// File System
void readSector(char *buffer, int sector);
void writeSector(char *buffer, int sector);
void readFile(char *buffer, char *path, int *result, char parentIndex);
void writeFile(char *buffer, char *path, int *result, char parentIndex);
void deleteFile(char *path, int *result, char parentIndex); 
void makeDirectory(char *path, int *result, char parentIndex);
void deleteDirectory(char *path, int *success, char parentIndex); 
// Execute a Program
void putArgs (char curdir, char argc, char **argv);
void getCurdir (char *curdir);
void getArgc (char *argc);
void getArgv (char index, char *argv);
void yieldControl ();
void sleep ();
void pauseProcess (int segment, int *result);
void resumeProcess (int segment, int *result);
void killProcess (int segment, int *result);
void executeProgram (char *path, int asBackground, int *result, char parentIndex);
void terminateProgram(int* result); 
void showProcess();

int main() {	
	int result;
	initializeProcStructures();
	makeInterrupt21();
	makeTimerInterrupt();
	interrupt(0x21, 0xFF << 8 | 0x06, "shell", 0, &result);
}

void handleInterrupt21 (int AX, int BX, int CX, int DX) {
	char AL, AH;
	AL = (char) (AX);
	AH = (char) (AX >> 8);

	switch (AL) {
		case 0x00: printString(BX); break;
		case 0x01: readString(BX, CX); break;
		case 0x02: readSector(BX, CX); break;
		case 0x03: writeSector(BX, CX); break;
		case 0x04: readFile(BX, CX, DX, AH); break;
		case 0x05: writeFile(BX, CX, DX, AH); break;
		case 0x06: executeProgram(BX, CX, DX, AH); break;
		case 0x07: terminateProgram(BX); break;
		case 0x08: makeDirectory(BX, CX, AH); break;
		case 0x09: deleteFile(BX, CX, AH); break;
		case 0x0A: deleteDirectory(BX, CX, AH); break;
		case 0x20: putArgs(BX, CX, DX); break;
		case 0x21: getCurdir(BX); break;
		case 0x22: getArgc(BX); break;
		case 0x23: getArgv(BX, CX); break;
		case 0x31: sleep(); break;
		case 0x32: pauseProcess(BX, CX); break;
		case 0x33: resumeProcess(BX, CX); break;
		case 0x34: killProcess(BX, CX); break;
		case 0x35: showProcess(); break;
		default: printString("Invalid interrupt");
	}
}

void handleTimerInterrupt(int segment, int stackPointer) {
	struct PCB *currPCB;
	struct PCB *nextPCB;
	setKernelDataSegment();
	currPCB = getPCBOfSegment(segment);
	currPCB->stackPointer = stackPointer;
	if (currPCB->state != PAUSED) {
		currPCB->state = READY;
		addToReady(currPCB);
	}
	do {
		nextPCB = removeFromReady();
	} while (nextPCB != NULL && (nextPCB->state == DEFUNCT || nextPCB->state == PAUSED));
	if (nextPCB != NULL) {
		nextPCB->state = RUNNING;
		segment = nextPCB->segment;
		stackPointer = nextPCB->stackPointer;
		running = nextPCB;
	} else running = &idleProc;
	restoreDataSegment();
	returnFromTimer(segment, stackPointer);
}

//=======================================================================================
// Utility
//=======================================================================================

void printString(char *string) { // Works like println
	int i = 0;
	while (string[i] != '\0') interrupt(0x10, 0xE00 + string[i++], 0, 0, 0);
}

void readString(char *string, int disableProcessControl) {
	int index = 0;
	int result;
	char input_buffer = 0x00; // To remember last input
	do {
		input_buffer = interrupt(0x16, 0, 0, 0, 0); // Read a character from the keyboard
		if (input_buffer == '\r') { // If it's an ENTER, terminate it with a NULL
			interrupt(0x10, 0xE00 + input_buffer, 0, 0, 0); // Print it
			string[index] = '\0'; 
		} else if (input_buffer == 0x03 && !disableProcessControl) { // ctrl c
			setKernelDataSegment();
			printString("\r\nProcess killed\r\n");			
			restoreDataSegment();
			terminateProgram(&result);			
		} else if (input_buffer == 0x1A && !disableProcessControl) { // ctrl z
			setKernelDataSegment();		
			printString("\r\nProcess suspended\r\n");		
			restoreDataSegment();				
			sleep();
			resumeProcess(0x2000, result); // Resume shell		
		}
		else if (input_buffer != '\b') { // If it's not a backspace, input it
			interrupt(0x10, 0xE00 + input_buffer, 0, 0, 0); // Print it
			string[index++] = input_buffer; 
		} else if (index > 0) { // If it is backspace, space over the last character
			--index;
			interrupt(0x10, 0xE00 + input_buffer, 0, 0, 0); // Print it
			interrupt(0x10, 0xE00 + 0x20, 0, 0, 0);
			interrupt(0x10, 0xE00 + '\b', 0, 0, 0);
		}
	} while (input_buffer != '\r'); // Wait for ENTER 
	interrupt(0x10, 0xE00 + '\n', 0, 0, 0);
}

#include "../library/math.h"
#include "../library/strutils.h"
#include "../library/fsutils.h"

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
	char sectors[SECTOR_SIZE];
	int dirs_offset = 0, dirsname_offset = 0, filename_idx = 0, files_offset = 0, sectors_offset = 0;
	// Find the index of first character of the filename, to determine when to search for the filename instead of dirsname
	while (path[dirsname_offset] != '\0') {	
		if (path[dirsname_offset] == '/') filename_idx = dirsname_offset + 1;
		++dirsname_offset;
	}
	dirsname_offset = 0;
	// Search for path
	if (filename_idx != 0) {
		path[filename_idx - 1] = '\0'; // cut the slash
		dirs_offset = searchPath(path, parentIndex);
		if (dirs_offset == 0xFE) {*result = NOT_FOUND; return;}
	} else dirs_offset = parentIndex;
	// Search for the file in the path
	files_offset = searchFile(path + filename_idx, dirs_offset);
	if (files_offset == MAX_FILES) {*result = NOT_FOUND; return;}	
	// Read the file from its sectors
	readSector(sectors, SECTORS_SECTOR);
	while(sectors[files_offset * SECTORS_ENTRY_LENGTH + sectors_offset] != '\0' && sectors_offset < SECTORS_ENTRY_LENGTH) {
		readSector(buffer + sectors_offset * SECTOR_SIZE, sectors[files_offset * SECTORS_ENTRY_LENGTH + sectors_offset]);
		++sectors_offset;
	} 
	*result = files_offset;
}

void writeFile(char *buffer, char *path, int *result, char parentIndex) {
	char map[SECTOR_SIZE], files[SECTOR_SIZE], sectors[SECTOR_SIZE];
	int map_offset = 0;
	int dirs_offset = 0, dirsname_offset = 0;
	int files_offset = 0, filesname_offset = 0, empty_files_index = 0;
	int sectors_offset = 0, empty_sector = 0;
	int filename_idx = 0;
	// Search for empty sector
	readSector(map, MAP_SECTOR);
	map_offset = findUnusedSector(map);
	// If there's no empty sectors
	if (map_offset == NOT_FOUND) {
		*result = INSUFFICIENT_MEMORY;
		return;
	}
	// Search for empty file entry
	readSector(files, FILES_SECTOR);
	for (empty_files_index = 0; empty_files_index < MAX_FILES; ++empty_files_index) {
		if (files[empty_files_index * DIRS_ENTRY_LENGTH + 1] == '\0') break;
	}
	// If there's no empty entries
	if (files_offset == MAX_FILES) {
		*result = INSUFFICIENT_MEMORY;
		return;
	}
	// Find the index of first character of the filename, to determine when to search for the filename instead of dirsname
	while (path[dirsname_offset] != '\0') {	
		if (path[dirsname_offset] == '/') filename_idx = dirsname_offset + 1;
		++dirsname_offset;
	}
	dirsname_offset = 0;
	// Search for path
	if (filename_idx != 0) { 
		path[filename_idx - 1] = '\0';		
		dirs_offset = searchPath(path, parentIndex);
		if (dirs_offset == 0xFE) {*result = NOT_FOUND; return;}
	} else dirs_offset = parentIndex;
	// Search whether the file exists or not
	files_offset = searchFile(path + filename_idx, dirs_offset);
	if (files_offset != MAX_FILES) {*result = ALREADY_EXISTS; return;}
	// Write to file entry
	files[empty_files_index * FILES_ENTRY_LENGTH] = (char) dirs_offset;
	dirsname_offset = 0;
	while (path[filename_idx + dirsname_offset] != '\0') {
		files[empty_files_index * FILES_ENTRY_LENGTH + dirsname_offset + 1] = path[filename_idx + dirsname_offset];	
		++dirsname_offset;
	}
	// Write buffer
	readSector(sectors, SECTORS_SECTOR);	
	while (buffer[sectors_offset * SECTOR_SIZE] != '\0') {
		empty_sector = findUnusedSector(map);
		writeSector(buffer + sectors_offset * SECTOR_SIZE, empty_sector);
		sectors[empty_files_index * SECTORS_ENTRY_LENGTH + sectors_offset] = empty_sector;
		map[empty_sector] = 0xFF;
		++sectors_offset;
	}
	// Write to floppy
	writeSector(map, MAP_SECTOR);
	writeSector(files, FILES_SECTOR);
	writeSector(sectors, SECTORS_SECTOR);
	*result = SUCCESS;
}

void deleteFile(char *path, int *result, char parentIndex) {
	char map[SECTOR_SIZE], files[SECTOR_SIZE], sectors[SECTOR_SIZE];
	int dirs_offset = 0, dirsname_offset = 0, filename_idx = 0;
	int files_offset = 0;
	int sectors_offset = 0;
	
	// Find the index of the last slash, to determine when to search for the filename instead of dirsname
	while (path[dirsname_offset] != '\0') {	
		if (path[dirsname_offset] == '/') filename_idx = dirsname_offset + 1;
		++dirsname_offset;
	}
	// Search for path
	if (filename_idx != 0) {
		path[filename_idx - 1] = '\0';
		dirs_offset = searchPath(path, parentIndex);
		if (dirs_offset == 0xFE) {*result = NOT_FOUND; return;}
	} else dirs_offset = parentIndex;
	// Search for the file
	files_offset = searchFile(path + filename_idx, dirs_offset);
	if (files_offset == MAX_FILES) {*result = NOT_FOUND; return;}
	// Delete in map
	readSector(map, MAP_SECTOR);
	readSector(sectors, SECTORS_SECTOR);
	sectors_offset = 0;
	while (sectors[files_offset * SECTORS_ENTRY_LENGTH + sectors_offset] != '\0') {
		map[sectors[files_offset * SECTORS_ENTRY_LENGTH + sectors_offset]] = 0x00;
		// clear sectors?
		++sectors_offset;
	}
	// Delete file entry
	readSector(files, FILES_SECTOR);
	files[files_offset * FILES_ENTRY_LENGTH + 1] = '\0';
	// Write buffer
	writeSector(map, MAP_SECTOR);
	writeSector(files, FILES_SECTOR);
	writeSector(sectors, SECTORS_SECTOR);
	*result = SUCCESS;
}


void makeDirectory(char *path, int *result, char parentIndex) {
	char dirs[SECTOR_SIZE], sectors[SECTOR_SIZE];
	int i;
	int dirsOffset = 0, dirsNameOffset = 0, target_dir_idx = 0;
	int unusedEntry, toBeCreatedOffset = 0;
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
			target_dir_idx = dirsNameOffset + 1;
		}
		dirsNameOffset += 1;
	}
	// Search for parent path
	if (target_dir_idx != 0) {
		path[target_dir_idx - 1] = '\0';
		dirsOffset = searchPath(path, parentIndex);
		if (dirsOffset == 0xFE) {*result = NOT_FOUND; return;}
	} else dirsOffset = parentIndex;
	// Check the availability of path name
	toBeCreatedOffset = searchPath(path + target_dir_idx, dirsOffset);
	if (toBeCreatedOffset != 0xFE) {*result = ALREADY_EXISTS; return;}
	
	// No dirs yet, write directory
	dirs[unusedEntry * DIRS_ENTRY_LENGTH] = dirsOffset;
	i = 1;
	for(dirsNameOffset = target_dir_idx; path[dirsNameOffset] != '\0'; ++dirsNameOffset) {
		dirs[unusedEntry * DIRS_ENTRY_LENGTH + i] = path[dirsNameOffset];
		i++;
	}
	writeSector(dirs,DIRS_SECTOR);
	*result = SUCCESS;	
}

void deleteDirectory(char *path, int *success, char parentIndex) {
	char dirs[SECTOR_SIZE], files[SECTOR_SIZE];
	int dirdelOffset = 0;
	int dirsOffset = 0;
	int files_offset = 0;
	char* delresult;
	readSector(dirs,DIRS_SECTOR);
	// Search for path
	dirsOffset = searchPath(path, parentIndex);
	if (dirsOffset == 0xFE) {*success = NOT_FOUND; return;}
	// Delete the file inside the dir, if any
	readSector(files, FILES_SECTOR);
	for(files_offset = 0; files_offset < MAX_FILES; ++files_offset) {		
		if (files[files_offset * FILES_ENTRY_LENGTH] == dirsOffset && files[files_offset * FILES_ENTRY_LENGTH + 1] != '\0') {
			deleteFile(files + files_offset * FILES_ENTRY_LENGTH + 1, delresult, dirsOffset);
		}
	}
	// Delete the dir inside the dir, if any
	for(dirdelOffset = 0; dirdelOffset < MAX_FILES; ++dirdelOffset) {
		if(dirs[dirdelOffset * DIRS_ENTRY_LENGTH] == dirsOffset && dirs[dirdelOffset * DIRS_ENTRY_LENGTH + 1] != '\0') {
			deleteDirectory(dirs + dirdelOffset * DIRS_ENTRY_LENGTH + 1, delresult, dirsOffset);
			readSector(dirs,DIRS_SECTOR);
		}
	}
	// Delete the dir
	dirs[dirsOffset * DIRS_ENTRY_LENGTH + 1] = '\0';
	writeSector(dirs, DIRS_SECTOR);	
	*success = SUCCESS;
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
	for (p = 2; p < ARGS_SECTOR && i < argc; ++p) {
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
	for (p = 2; p < ARGS_SECTOR; ++p) {
		if (i == index) {
			argv[j] = args[p];
			++j;
		}
		if (args[p] == '\0') {
			if (i == index) break; else ++i;
		}
	}
}

void yieldControl () {interrupt(0x08, 0, 0, 0, 0);}

void sleep () {
	setKernelDataSegment();
	running->state = PAUSED;
	restoreDataSegment();
	yieldControl();
}

void pauseProcess (int segment, int *result) {
	struct PCB *pcb;
	int res;
	setKernelDataSegment();
	pcb = getPCBOfSegment(segment);
	if (pcb != NULL && pcb->state != PAUSED) {
		pcb->state = PAUSED;
		res = SUCCESS;
	} else res = NOT_FOUND;
	restoreDataSegment();
	*result = res;
}

void resumeProcess (int segment, int *result) {
	struct PCB *pcb;
	int res;
	setKernelDataSegment();
	pcb = getPCBOfSegment(segment);
	if (pcb != NULL && pcb->state == PAUSED) {
		pcb->state = READY;
		addToReady(pcb);
		res = SUCCESS;
	} else res = NOT_FOUND;
	restoreDataSegment();
	*result = res;
}

void killProcess (int segment, int *result) {
	struct PCB *pcb;
	int res;
	setKernelDataSegment();
	pcb = getPCBOfSegment(segment);
	if (pcb != NULL) {
		releaseMemorySegment(pcb->segment);
		releasePCB(pcb);
		res = SUCCESS;
	} else res = NOT_FOUND;
	restoreDataSegment();
	*result = res;
}

void executeProgram (char *path, int asBackground, int *result, char parentIndex) {
	struct PCB* pcb;
	int segment;
	int i, fileIndex;
	char buffer[MAX_SECTORS * SECTOR_SIZE];
	readFile(buffer, path, result, parentIndex);
	if (*result != NOT_FOUND) {
		setKernelDataSegment();
		segment = getFreeMemorySegment();
		restoreDataSegment();
		fileIndex = *result;
		if (segment != NO_FREE_SEGMENTS) {
			setKernelDataSegment();
			pcb = getFreePCB();
			pcb->index = fileIndex;
			pcb->state = STARTING;
			pcb->segment = segment;
			pcb->stackPointer = 0xFF00;
			pcb->parentSegment = running->segment;
			addToReady(pcb);
			restoreDataSegment();
			for (i = 0; i < SECTOR_SIZE * MAX_SECTORS; i++) {
				putInMemory(segment, i, buffer[i]);
			}
			initializeProgram(segment);
			if (!asBackground) sleep(); // If executed as background, don't sleep the parent process
		} else *result = INSUFFICIENT_MEMORY;
	}
}

void terminateProgram (int *result) {
	int parentSegment;
	setKernelDataSegment();
	parentSegment = running->parentSegment;
	releaseMemorySegment(running->segment);
	releasePCB(running);
	restoreDataSegment();
	if (parentSegment != NO_PARENT) resumeProcess(parentSegment, result);
	yieldControl();
}

void showProcess() {
	char files[SECTOR_SIZE];
	int i;
	char temp1;
	unsigned int temp2;
	char* temp3;
	readSector(files, FILES_SECTOR);
	setKernelDataSegment();
	for(i = 0; i < MAX_SEGMENTS; ++i) {
		if (memoryMap[i] == SEGMENT_USED) {
			temp1 = pcbPool[i].index;
			temp3 = "  |  ";
			restoreDataSegment();
			setKernelDataSegment();
			temp2 = (pcbPool[i].segment >> 12) - 2;
			printInt(temp2);
			restoreDataSegment();
			setKernelDataSegment();
			printString(temp3);
			restoreDataSegment();
			readSector(files, FILES_SECTOR);
			printString(files + temp1 * FILES_ENTRY_LENGTH + 1);
			setKernelDataSegment();
			printString("\r\n");
			restoreDataSegment();
			setKernelDataSegment();
		}
	}
	restoreDataSegment();
}