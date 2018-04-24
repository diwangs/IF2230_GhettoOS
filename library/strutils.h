#ifndef STRUTILS_H
#define STRUTILS_H
#include "math.h"
// Biar shell dan kernel bisa #include
int strcmp(char* s1, char* s2) {
	int i = 0;
	while (!(s1[i] == '\0' && s2[i] == '\0')) {
		if (s1[i] != s2[i]) return 0;
		++i;
	}
	return 1;
}

void strcpy(char* sc, char* sd) {
	int i = 0;
	clear(sd, 15);
	do {
		sd[i] = sc[i];
		++i;
	} while(sc[i] != '\0');
}

void printInt(int i) {
	char ir = '0' + (char) div(i, 100);
	char ip = '0' + (char) div(mod(i, 100), 10);
	char is = '0' + (char) mod(i, 10);
	interrupt(0x10, 0xE00 + ir, 0, 0, 0);
	interrupt(0x10, 0xE00 + ip, 0, 0, 0);
	interrupt(0x10, 0xE00 + is, 0, 0, 0);
}

void clear(char *buffer, int length) {
	int i;
	for (i = 0; i < length; ++i) buffer[i] = '\0';
}
#endif