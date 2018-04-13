void main() {
	char curdir;
	char argc;
	char *argv[100], succ;
	interrupt(0x21, 0x21, &curdir, 0, 0);
	interrupt(0x21, 0x22, &argc, 0, 0);
	if (argc > 0) {
		interrupt(0x21, 0x23, 0, argv[0], 0);
		// Try deleting file
		interrupt(0x21, curdir << 8 | 0x09, argv[0], &succ, 0);
		if (succ != 0)
			// Try deleting dir
			interrupt(0x21, curdir << 8 | 0x0A, argv[0], &succ, 0);
		if (succ != 0)
			interrupt(0x21, 0x00, "File or directory not found\r\n", 0, 0);
		else interrupt(0x21, 0x00, "Success deleting\r\n", 0, 0);
	}
	interrupt(0x21, (curdir << 8) | 0x06, "shell", 0x2000, &succ);
}