int main() {
	char curdir, argc, res;
	char argv[20];
	char isifile[10];
	int *result, i;

	// get curdir
	interrupt(0x21, 0x21, &curdir, 0, 0);
	// get argc
	interrupt(0x21, 0x22, &argc, 0, 0);
	// get arguement(s)
	interrupt(0x21, 0x23, 0, argv, 0);

	if (argc == 2) {	// menuliskan isi file
		// baca input user
		interrupt(0x21, 0x00, "Masukan isi file: \r\n", 0, 0);
		interrupt(0x21, 0x01, isifile, 0, 0);			
		// masukkan input user ke file yang dituju
		interrupt(0x21, 0xFF << 8 | 0x05, isifile, argv, result);
		
		// jika berhasil, cetak pesan berhasil
		if (result == 0) {
			interrupt(0x21, 0x00, "File berhasil ditulis\r\n", 0, 0);
		}
	} else {			// menampilkan isi file
		// asumsi file dipastikan ada di di curent directory
		// readfile
		interrupt(0x21, (curdir << 8) | 0x04, isifile, argv, result);
		// print isi ke layar
		if (result == 0) {
			interrupt(0x21, 0x00, isifile, 0, 0);
			interrupt(0x21, 0x00, "\r\n", 0, 0);			
		}
	}

	interrupt(0x21, (curdir << 8) | 0x06, "shell", 0x2000, result);
}