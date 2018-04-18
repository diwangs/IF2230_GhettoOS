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

void strcpy(char* sc, char* sd) {
	int i = 0;
	clear(sd, 15);
	do {
		sd[i] = sc[i];
		++i;
	} while(sc[i] != '\0');
}

void clear(char *buffer, int length) {
	int i;
	for (i = 0; i < length; ++i) buffer[i] = '\0';
}
#endif