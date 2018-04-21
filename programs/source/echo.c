#include "../../library/declaration/math_dec.h"
void printInt(int i);

int main(){
    char curdir, res;
    char *argv[100];
    char argc;

    interrupt(0x21, 0x21, &curdir, 0, 0);

    interrupt(0x21, 0x22, &argc, 0, 0);
    // printInt(argc);

    interrupt(0x21, 0x23, 0, argv[0], 0);
    interrupt(0x21, 0x0, argv[0], 0, 0);
    interrupt(0x21, 0x0, "\r\n", 0, 0);
    interrupt(0x21, (0xFF << 8) | 0x06, "shell", 0x2000, &res);
}

#include "../../library/math.h"
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