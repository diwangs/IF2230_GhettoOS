#include "../../library/declaration/math_dec.h"
#include "../../library/declaration/strutils_dec.h"
#include "../../library/declaration/fsutils_dec.h"
void printInt(int i);

int main() {
	char curdir, argc, res;
	char **argv;
	char *isifile;
	int result, i;

	// for checking
	char checkpath;

	// get curdir
	interrupt(0x21, 0x21, &curdir, 0, 0);
	// get argc
	interrupt(0x21, 0x22, &argc, 0, 0);
	// get arguement(s)
	for (i = 0; i < argc; ++i) {
		interrupt(0x21, 0x23, i, argv[i], 0);
		// interrupt(0x21, 0x00, "mengisi variable\r\n", 0, 0);
	}

	// interrupt(0x21, 0x00, "argc: ", 0 ,0);
	// printInt(argc);
	interrupt(0x21, 0x00, "file yang dituju: ", 0, 0);
	interrupt(0x21, 0x00, argv[0], 0, 0);
	interrupt(0x21, 0x00, "\r\n", 0, 0);
	// interrupt(0x21, 0x00, "\r\n\n", 0, 0);

	if (argc == 2 && strcmp(argv[1],"-w") == 1) {	// menuliskan isi file
		// baca input user
		interrupt(0x21, 0x00, "Masukan isi file: ", 0, 0);
		interrupt(0x21, 0x01, isifile, 0, 0);
		// masukkan input user ke file yang dituju
		interrupt(0x21, curdir << 8 | 0x05, isifile, argv[0], &result);

		// checking
		interrupt(0x21, 0x00, isifile, 0, 0);
		interrupt(0x21, 0x00, "\r\n", 0, 0);
		checkpath = searchFile("afile", curdir);
		if (checkpath == 32) {
			interrupt(0x21, 0x00, "ndak ada\r\n", 0, 0);
		} else {
			interrupt(0x21, 0x00, "file ada", 0, 0);
		}
		
		// jika berhasil, cetak pesan berhasil
		if (result == 0) {
			interrupt(0x21, 0x00, "File berhasil ditulis\r\n", 0, 0);
		}
	} else {			// menampilkan isi file
		// asumsi file dipastikan ada di di curent directory
		// readfile
		interrupt(0x21, (curdir << 8) | 0x04, isifile, argv[0], &result);
		// print isi ke layar
		if (result == 0) {
			interrupt(0x21, 0x00, isifile, 0, 0);
		}
	}

	// kembali ke shell
	// result = 0;
	// interrupt(0x21, 0x07, &result, 0, 0);
	interrupt(0x21, (curdir << 8) | 0x06, "shell", 0x2000, &res);
}

#include "../../library/math.h"
#include "../../library/strutils.h"
#include "../../library/fsutils.h"
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