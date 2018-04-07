/* int main() {
	 putInMemory(0xB000, 0x8000, 'K');
 }

// void handleInterrupt21 (int AX, int BX, int CX, int DX) {}*/

#define MAX_BYTE 256
#define SECTOR_SIZE 512
#define MAX_FILES 16
// #define MAX_FILENAME 12
#define MAX_FILENAME 15
#define MAX_SECTORS 20
#define DIR_ENTRY_LENGTH 32
#define MAP_SECTOR 1
#define DIR_SECTOR 2
#define TRUE 1
#define FALSE 0
#define INSUFFICIENT_SECTORS 0
#define NOT_FOUND -1
#define INSUFFICIENT_DIR_ENTRIES -1
#define EMPTY 0x00
#define USED 0xFF
#define ARGS_SECTOR 512

void handleInterrupt21(int AX, int BX, int CX, int DX);
void printString(char *string);
void readString(char *string);
int mod(int a, int b);
int div(int a, int b);
void readSector(char *buffer, int sector);
void writeSector(char *buffer, int sector);
// void readFile(char *buffer, char *filename, int *success);
void readFile(char *buffer, char *path, int *result, char parentIndex);
void clear(char *buffer, int length);
// void writeFile(char *buffer, char *filename, int *sectors);
void writeFile(char *buffer, char *path, int *sectors, char parentIndex);
// void executeProgram(char *filename, int segment, int *success);
void executeProgram(char *path, int segment, int *result, char parentIndex);
void printLogo();
void terminateProgram(int *result);
void makeDirectory(char *path, int *result, char parentIndex);
void deleteFile(char *path, int *result, char parentIndex);
void deleteDirectory(char *path, int *success, char parentIndex);

int main() {		
	makeInterrupt21();	
	printLogo();
	printString("sus");
	while(1) {}
}

void handleInterrupt21 (int AX, int BX, int CX, int DX) {
	// pemanggilan interrupt: interrupt(0x21, (AH << 8) | AL, BX, CX, DX);

	char AL, AH;
	AL = (char) (AX);
	AH = (char) (AX >> 8);

	switch (AL) {
		case 0x00:
			printString(BX);
			break;
		case 0x01:
			readString(BX);
			break;
		case 0x02:
			readSector(BX, CX);
			break;
		case 0x03:
			writeSector(BX, CX);
			break;
		case 0x04:
			readFile(BX, CX, DX, AH);
			break;
		case 0x05:
			writeFile(BX, CX, DX, AH);
			break;
		case 0x06:
			executeProgram(BX, CX, DX, AH);
			break;
		case 0x07:
			terminateProgram(BX);
			break;
		case 0x08:
			makeDirectory(BX, CX, AH);
			break;
		case 0x09:
			deleteFile(BX, CX, AH);
			break;
		case 0x0A:
			deleteDirectory(BX, CX, AH);
			break;
		case 0x20:
			putArgs(BX, CX);
			break;
		case 0x21:
			getCurdir(BX);
			break;
		case 0x22:
			getArgc(BX);
			break;
		case 0X23:
			getArgv(BX, CX);
			break;
		default:
			printString("Invalid interrupt");
	}
}

// void handleInterrupt21(int AX, int BX, int CX, int DX) { // asm linking
// 	switch (AX) {
// 		case 0x0:
// 			printString(BX);
// 			break;
// 		case 0x1:
// 			readString(BX);
// 			break;
// 		case 0x2:
// 			readSector(BX, CX);
// 			break;
// 		case 0x3:
// 			writeSector(BX, CX);
// 			break;
// 		case 0x4:
// 			readFile(BX, CX, DX);
// 			break;
// 		case 0x5:
// 			writeFile(BX, CX, DX);
// 			break;
// 		case 0x6:
// 			executeProgram(BX, CX, DX);
// 			break;
// 		default:
// 			printString("Invalid interrupt");
// 	}
// }

void printString(char *string) { // Works like println
	int i = 0;
	while (string[i] != '\0') interrupt(0x10, 0xE00 + string[i++], 0, 0, 0);
	interrupt(0x10, 0xE00 + '\r', 0, 0, 0);       
	interrupt(0x10, 0xE00 + '\n', 0, 0, 0);
}

void readString(char *string) {
	int index = 0;
	char input_buffer = 0x00; // To remember last input
	do {
		input_buffer = interrupt(0x16, 0, 0, 0, 0); // Read a character from the keyboard
		interrupt(0x10, 0xE00 + input_buffer, 0, 0, 0); // Print it
		if (input_buffer == '\r') string[index] = '\0'; // If it's an ENTER, terminate it with a NULL
		else if (input_buffer != '\b') string[index++] = input_buffer; // If it's not a backspace, input it
		else if (index-- > 0) { // If it is backspace, space over the last character
			interrupt(0x10, 0xE00 + 0x20, 0, 0, 0);
			interrupt(0x10, 0xE00 + '\b', 0, 0, 0);
		}
	} while (input_buffer != '\r'); // Wait for ENTER 
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

void readSector(char *buffer, int sector) {
	interrupt(0x13, 0x201, buffer, div(sector, 36) * 0x100 + mod(sector, 18) + 1, mod(div(sector, 18), 2) * 0x100);
}

void writeSector(char *buffer, int sector) {
	interrupt(0x13, 0x301, buffer, div(sector, 36) * 0x100 + mod(sector, 18) + 1, mod(div(sector, 18), 2) * 0x100);
}

void readFile(char *buffer, char *path, int *result, char parentIndex) {
	/* buat versi baru */
	
	/* Untuk membaca file “g” dengan path “/abc/def/g” dapat dilakukan langkah-langkah berikut: */
	/* 1. Cari indeks direktori pada sektor ​dirs​​ yang bernama “abc” dan indeks parentnya adalah
	 * adalah parentIndex (0xFF jika root). Jika tidak ada maka syscall akan gagal dan
	 * mengembalikan error NOT_FOUND (-1) pada parameter result. */
	

	/* 2. Cari indeks direktori pada sektor ​dirs​​ yang bernama “def” dan indeks parentnya adalah
	 * indeks direktori “abc” yang didapatkan sebelumnya. Jika tidak ada maka syscall akan gagal
	 * dan mengembalikan error NOT_FOUND (-1) pada parameter result. */

	/* 3. Cari indeks file pada sektor ​files​​ yang bernama “g” dan indeks parentnya adalah indeks
	 * direktori “def” yang didapatkan sebelumnya. Jika tidak ada maka syscall akan gagal dan
	 * mengembalikan error NOT_FOUND (-1) pada parameter result. */

	/* 4. Baca daftar sektor file pada entry file dengan indeks tersebut pada sektor ​sectors​​. */

	/* 5. Masukkan sektor-sektor tersebut ke buffer. */

	/* 6. Kembalikan success (0) pada parameter result */
}

// void readFile(char *buffer, char *filename, int *success) {
// 	char dir[SECTOR_SIZE];
// 	int found = 0, i = 0, j = 0;		
// 	readSector(dir, DIR_SECTOR);
// 	// Search the sector for the file
// 	while (!found && i * DIR_ENTRY_LENGTH < SECTOR_SIZE) {
// 		// Match the filename
// 		found = 1;
// 		for (j = 0; j < MAX_FILES && filename[j] != '\0'; ++j) {
// 			if (dir[i * DIR_ENTRY_LENGTH + j] != filename[j]) {
// 				found = 0;
// 				break;
// 			} 
// 		}
// 		if(!found) ++i;
// 	}
// 	// If the file is not found
// 	if (!found) {
// 		*success = 0;
// 		return;
// 	}
// 	// Else, read the file sector
// 	for (j = 0; j < MAX_SECTORS && dir[i * DIR_ENTRY_LENGTH + MAX_FILENAME + j] != '\0'; ++j) {	
// 		readSector(buffer + j * SECTOR_SIZE, dir[i * DIR_ENTRY_LENGTH + MAX_FILENAME + j]);
// 	}	
// 	*success = 1;
// 	return;
// }

void clear(char *buffer, int length) {
	int i;
	for (i = 0; i < length; ++i) buffer[i] = EMPTY;
}

void writeFile(char *buffer, char *path, int *sectors, char parentIndex) {
	/* buat versi sendiri */

	/* Untuk menulis file “g” dengan path “/abc/def/g” dapat dilakukan langkah-langkah berikut: */
	/* 1. Cek apakah jumlah sektor kosong cukup pada sektor ​map​​. Jika tidak ada maka syscall
	 * akan gagal dan mengembalikan error INSUFFICIENT_SECTORS (0) pada parameter sectors. */
	/* 2. Cek apakah masih tersisa entry kosong pada sektor ​files​​ (ciri-ciri entry kosong adalah byte
	 * pertama dari nama filenya adalah karakter NUL ‘\0’). Jika tidak ada maka syscall akan gagal
	 * dan mengembalikan error INSUFFICIENT_ENTRIES (-3) pada parameter sectors. */
	/* 3. Cari indeks direktori pada sektor ​dirs​​ yang bernama “abc” dan indeks parentnya adalah
	 * adalah parentIndex (0xFF jika root). Jika tidak ada maka syscall akan gagal dan
	 * mengembalikan error NOT_FOUND (-1) pada parameter sectors. */
	/* 4. Cari indeks direktori pada sektor ​dirs​​ yang bernama “def” dan indeks parentnya adalah
	 * indeks direktori “abc” yang didapatkan sebelumnya. Jika tidak ada maka syscall akan gagal
	 * dan mengembalikan error NOT_FOUND (-1) pada parameter sectors. */
	/* 5. Cari indeks file pada sektor ​files​​ yang bernama “g” dan indeks parentnya adalah indeks
	 * direktori “def” yang didapatkan sebelumnya. Jika ada maka syscall akan gagal dan
	 * mengembalikan error ALREADY_EXISTS (-2) pada parameter sectors. */
	/* 6. Pada entry kosong pertama pada sektor ​files​​ tulis indeks dari direktori “def” yang
	 * didapatkan sebelumnya pada byte indeks parent dan “g” pada byte-byte nama filenya. */
	/* 7. Cari sektor kosong pertama pada sektor ​map​​ dan mark byte sektor tersebut menjadi terisi (0xFF). */
	/* 8. Tulis data dari buffer ke sektor dengan nomor tersebut. */
	/* 9. Lakukan 6 dan 7 untuk setiap sektor dari file. */
}

// void writeFile(char *buffer, char *filename, int *sectors) {
// 	char map[SECTOR_SIZE];
// 	char dir[SECTOR_SIZE];
// 	char sectorBuffer[SECTOR_SIZE];
// 	int dirIndex;
// 	readSector(map, MAP_SECTOR);
// 	readSector(dir, DIR_SECTOR);
// 	for (dirIndex = 0; dirIndex < MAX_FILES; ++dirIndex) {
// 		if (dir[dirIndex * DIR_ENTRY_LENGTH] == '\0') break;
// 	}
// 	if (dirIndex < MAX_FILES) {
// 		int i, j, sectorCount;
// 		for (i = 0, sectorCount = 0; i < MAX_BYTE && sectorCount < *sectors; ++i) {
// 			if (map[i] == EMPTY) ++sectorCount;
// 		}
// 		if (sectorCount < *sectors) {
// 			*sectors = INSUFFICIENT_SECTORS;
// 			return;
// 		} else {
// 			clear(dir + dirIndex * DIR_ENTRY_LENGTH, DIR_ENTRY_LENGTH);
// 			for (i = 0; i < MAX_FILENAME; ++i) {
// 				if (filename[i] != '\0') {
// 					dir[dirIndex * DIR_ENTRY_LENGTH + i] = filename[i];
// 				} else break;
// 			}
// 			for (i = 0, sectorCount = 0; i < MAX_BYTE && sectorCount < *sectors; ++i) {
// 				if (map[i] == EMPTY) {
// 					map[i] = USED;
// 					dir[dirIndex * DIR_ENTRY_LENGTH + MAX_FILENAME +
// 						sectorCount] = i;
// 					clear(sectorBuffer, SECTOR_SIZE);
// 					for (j = 0; j < SECTOR_SIZE; ++j) {
// 						sectorBuffer[j] = buffer[sectorCount * SECTOR_SIZE + j];
// 					}
// 					writeSector(sectorBuffer, i);
// 					++sectorCount;
// 				}
// 			}
// 		}
// 	} else {
// 		*sectors = INSUFFICIENT_DIR_ENTRIES;
// 		return;
// 	}
// 	writeSector(map, MAP_SECTOR);
// 	writeSector(dir, DIR_SECTOR);
// }

void executeProgram(char *path, int segment, int *result, char parentIndex) { /* buat versi sendiri */ }

// void executeProgram(char *filename, int segment, int *success) {
// 	char buffer[MAX_SECTORS * SECTOR_SIZE];	
// 	int i;				
// 	readFile(buffer, filename, success); 
// 	if (!*success) return; 
// 	for (i = 0; i < MAX_SECTORS * SECTOR_SIZE; ++i) putInMemory(segment, i, buffer[i]);			
// 	launchProgram(segment); 
// }

void printCenter(int row, int ln, char* s){
   int i = 0;
   int offset = 0x8000 + ((row-1)*80*2) + (40-ln/2)*2;
   while(s[i] != '\0'){
      putInMemory(0xB000, offset + i*2, s[i]);
      i++;
   }
}

void printLogo(){
	int i = 0;
	
	// clear the screen
	while (i < 4000){
		putInMemory(0xB000, 0x8000 + i*2, '\0');
		i++;
	}

	// print logo in center
	printCenter(7, 20, "========        ");
   	printCenter(8, 20, "==               ");
   	printCenter(9, 20, "==   ===        ");
   	printCenter(10, 20,"==    ==        ");
   	printCenter(11, 20,"========        ");
   	printCenter(12, 20,"  / \\   ==   ==");
   	printCenter(13, 20," /   \\  ==   ==");
   	printCenter(14, 20,"/ 0 0 \\ =======");
   	printCenter(15, 20,"\\  .  / =======");
   	printCenter(16, 20," \\ v /  ==   ==");
   	printCenter(17, 20,"  \\ /   ==   ==");
   	printCenter(18, 20," Ghetto ==   ==");
	printString("\n\n\nPress any key to continue...");
   	interrupt(0x16,0,0,0,0);

}

void terminateProgram(int *result) { /* implementasi sendiri */ }

void makeDirectory(char *path, int *result, char parentIndex) {
	/* implementasi sendiri */
	
	/* Untuk membuat direktori “ghi” dengan path “/abc/def/ghi” dapat dilakukan langkah-langkah berikut: */
	/* 1. Cek apakah masih tersisa entry kosong pada sektor ​dirs​​ (ciri-ciri entry kosong adalah byte
	 * pertama dari nama direktorinya adalah karakter NUL ‘\0’). Jika tidak ada maka syscall akan
	 * gagal dan mengembalikan error INSUFFICIENT_ENTRIES (-3) pada parameter result. */
	/* 2. Cari indeks direktori pada sektor ​dirs​​ yang bernama “abc” dan indeks parentnya adalah
	 * adalah parentIndex (0xFF jika root). Jika tidak ada maka syscall akan gagal dan
	 * mengembalikan error NOT_FOUND (-1) pada parameter result. */
	/* 3. Cari indeks direktori pada sektor ​dirs​​ yang bernama “def” dan indeks parentnya adalah
	 * indeks direktori “abc” yang didapatkan sebelumnya. Jika tidak ada maka syscall akan gagal
	 * dan mengembalikan error NOT_FOUND (-1) pada parameter result. */
	/* 4. Cari indeks file pada sektor ​files​​ yang bernama “ghi” dan indeks parentnya adalah indeks
	 * direktori “def” yang didapatkan sebelumnya. Jika ada maka syscall akan gagal dan
	 * mengembalikan error ALREADY_EXISTS (-2) pada parameter result. */
	/* 5. Pada entry kosong pertama pada sektor ​dirs,​​ tulis indeks dari direktori “def” yang
	 * didapatkan sebelumnya pada byte indeks parent dan “ghi” pada byte-byte nama direktorinya. */
	/* 6. Kembalikan success (0) pada parameter result. */
}

void deleteFile(char *path, int *result, char parentIndex) {
	/* implementasi sendiri */

	/* Untuk menghapus file “g” dengan path “/abc/def/g” dapat dilakukan langkah-langkah berikut:
	 * 1. Cari indeks direktori pada sektor ​dirs​​ yang bernama “abc” dan indeks parentnya adalah
	 * parentIndex (0xFF jika root). Jika tidak ada maka syscall akan gagal dan mengembalikan
	 * error NOT_FOUND (-1) pada parameter result. */
	/* 2. Cari indeks direktori pada sektor ​dirs​​ yang bernama “def” dan indeks parentnya adalah
	 * indeks direktori “abc” yang didapatkan sebelumnya. Jika tidak ada maka syscall akan gagal
	 * dan mengembalikan error NOT_FOUND (-1) pada parameter result. */
	/* 3. Cari indeks file pada sektor ​files​​ yang bernama “g” dan indeks parentnya adalah indeks
	 * direktori “def” yang didapatkan sebelumnya. Jika tidak ada maka syscall akan gagal dan
	 * mengembalikan error NOT_FOUND (-1) pada parameter result. */
	/* 4. Pada entry file dengan indeks tersebut pada sektor ​files​​, ubah byte pertama nama filenya
	 * menjadi karakter NUL ‘\0’. */
	/* 5. Baca daftar sektor file pada entry file dengan indeks tersebut pada sektor ​sectors​​. */
	/* 6. Mark byte sektor-sektor tersebut menjadi kosong (0x00) pada sektor ​map​​. */
	/* 7. Kembalikan success (0) pada parameter result.	*/
}

void deleteDirectory(char *path, int *success, char parentIndex) {
	/* implementasi sendiri */

	/* Untuk menghapus direktori “ghi” dengan path “/abc/def/ghi” dapat dilakukan langkah-langkah berikut:
	/* 1. Cari indeks direktori pada sektor ​dirs​​ yang bernama “abc” dan indeks parentnya adalah
	 * adalah parentIndex (0xFF jika root). Jika tidak ada maka syscall akan gagal dan
	 * mengembalikan error NOT_FOUND (-1) pada parameter result. */
	/* 2. Cari indeks direktori pada sektor ​dirs​​ yang bernama “def” dan indeks parentnya adalah
	 * indeks direktori “abc” yang didapatkan sebelumnya. Jika tidak ada maka syscall akan gagal
	 * dan mengembalikan error NOT_FOUND (-1) pada parameter result. */
	/* 3. Cari indeks direktori pada sektor ​dirs​​ yang bernama “ghi” dan indeks parentnya adalah
	 * indeks direktori “def” yang didapatkan sebelumnya. Jika tidak ada maka syscall akan gagal
	 * dan mengembalikan error NOT_FOUND (-1) pada parameter result. */
	/* 4. Pada entry direktori dengan indeks tersebut pada sektor ​dirs​​, ubah byte pertama nama
	 * direktorinya menjadi karakter NUL ‘\0’. */
	/* 5. Cari indeks dari semua file yang indeks parentnya adalah indeks direktori “def” pada sektor
	 * files. ​​Hapus file-file tersebut menggunakan langkah 4-6 syscall deleteFile. Anda dapat
	 * memisahkan langkah-langkah tersebut menjadi fungsi deleteFile terpisah yang menerima
	 * indeks file langsung. */
	/* 6. Cari indeks dari semua direktori yang indeks parentnya adalah indeks direktori “def” pada
	 * sektor ​dirs​​. Hapus direktori-direktori tersebut menggunakan langkah 4-6 syscall
	 * deleteDirectory (termasuk langkah ini). Anda dapat memisahkan langkah-langkah tersebut
	 * menjadi fungsi deleteDirectory terpisah yang menerima indeks direktori langsung. */
	/* 7. Kembalikan success (0) pada parameter result. */
}

void putArgs (char curdir, char argc, char **argv) {
	char args[SECTOR_SIZE];
	int i, j, p;
	clear(args, SECTOR_SIZE);
	args[0] = curdir;
	args[1] = argc;
	i = 0;
	j = 0;
	for (p = 1; p < ARGS_SECTOR && i < argc; ++p) {
		args[p] = argv[i][j];
		if (argv[i][j] == '\0') {
			++i;
			j = 0;
		}
		else { ++j;	}
	}
	writeSector(args, ARGS_SECTOR);
}

void getCurdir (char *curdir) {
	char args[SECTOR_SIZE];
	readSector(args, ARGS_SECTOR);
	*curdir = args[0];
}

void getArgc (char *argc) {
	char args[SECTOR_SIZE];
	readSector(args, ARGS_SECTOR);
	*argc = args[1];
}

void getArgv (char index, char *argv) {
	char args[SECTOR_SIZE];
	int i, j, p;
	readSector(args, ARGS_SECTOR);
	i = 0;
	j = 0;
	for (p = 1; p < ARGS_SECTOR; ++p) {
		if (i == index) {
			argv[j] = args[p];
			++j;
		}
		if (args[p] == '\0') {
			if (i == index) { break; }
			else { ++i; }
		}
	}
}