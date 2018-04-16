int main() {
	char curdir, argc, res;
	char *argv;
	char *isifile;
	int result;

	// get curdir
	interrupt(0x21, 0x21, curdir, 0, 0);
	// get argc
	interrupt(0x21, 0x22, argc, 0, 0);
	// get arguement(s)
	interrupt(0x21, 0x23, 0, *argv, 0);

	if (argc == 1) {	// menampilkan isi file
		// asumsi file dipastikan ada di di curent directory
		// readfile
		interrupt(0x21, 0x04, (curdir << 8) | *isifile, *argv, &result);
		// print isi ke layar
		interrupt(0x21, 0x00, *isifile, 0, 0);
	} else {			// menuliskan ke file
		// baca input user
		interrupt(0x21, 0x00, "Masukan isi file: ", 0, 0);
		interrupt(0x21, 0x01, *isifile, 0, 0);
		// masukkan input user ke file yang dituju
		interrupt(0x21, 0x05, (curdir << 8) | *isifile, *argv, &result);
		// jika berhasil, cetak pesan berhasil
		if (result == 1) {
			interrupt(0x21, 0x00, "File berhasil ditulis\r\n", 0, 0);
		}
	}

	// kembali ke shell
	// result = 0;
	// interrupt(0x21, 0x07, &result, 0, 0);
	interrupt(0x21, (curdir << 8) | 0x06, "shell", 0x2000, &result);
}