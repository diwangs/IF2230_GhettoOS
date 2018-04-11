void printInt(int i);
int mod(int a, int b);
int div(int a, int b);

int main() {
    char dirname[100], argc;
    interrupt(0x21, 0x22, &argc, 0, 0);
    interrupt(0x21, 0x23, argc, dirname, 0);
    interrupt(0x21, 0x0, dirname, 0, 0);
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

int mod(int a, int b) { 
	while (a >= b) a -= b;
	return a;
}

int div(int a, int b) {
	int q = 0;
	while (q * b <= a) q += 1;
	return q - 1;
}