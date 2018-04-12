#ifndef STRUTILS_H
#define STRUTILS_H
// Biar shell dan kernel bisa #include
int strcmp(char* s1, char* s2) {
	int i = 0;
	while (!(s1[i] == '\0' && s2[i] == '\0')) {
		if (s1[i] != s2[i]) return 0;
		++i;
	}
	return 1;
}

void clear(char *buffer, int length) {
	int i;
	for (i = 0; i < length; ++i) buffer[i] = '\0';
}
#endif