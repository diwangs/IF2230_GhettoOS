#ifndef FSUTILS_DEC_H
#define FSUTILS_DEC_H

int findUnusedSector(char *map);
int findUnusedEntry(char *entries);
char searchPath(char* path, char parentIndex);
int searchFile(char* filename, char dir_index);

#endif