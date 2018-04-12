#ifndef FSUTILS_H
#define FSUTILS_H
#include "declaration/constants.h"
#include "strutils.h"

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

char searchPath(char* path, char parentIndex) { // return the index of the last dirs
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

char searchFile(char* filename, char dir_index) {
	char files[SECTOR_SIZE];
	int files_offset = 0, filesname_offset = 0;
    interrupt(0x21, 0x2, files, FILES_SECTOR); // readSector itu interrupt	
	do {
		// If the parent directory matches current parent...
		if (files[files_offset * FILES_ENTRY_LENGTH] == dir_index) { 
			// Match the file name
			if(strcmp(files + files_offset * FILES_ENTRY_LENGTH + 1, filename)) break;
		} 
		++files_offset;
	} while (files_offset < MAX_FILES);
	return files_offset;
}
#endif