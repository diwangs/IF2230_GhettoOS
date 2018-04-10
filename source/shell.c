/*** the kernel's shell ***/
int main() {
    char workingdir = 0xFF;            // current directory; default root
    char curdir = workingdir;          // directory yang dipakai untuk execute program
    
    /* belum ada implementasi command cd*/

    printString("$ ");
    char *input;
    readString(input);

    /*** memisahkan perintah dan argumen ***/
    char command[50];       // perintah
    char argc;              // jumlah argumen
    char argv[20][16];      // isi argumen (max 20)
    
    int i = 0;              // indeks string input
    int j = 0;
    if (input[i] != '\n') {     // ada perintah
        // memperoleh curdir
        if (input[i] == '.') {      // jika di current directory
            i++;
            curdir = workingdir;
        } else {                    // jika di root
            curdir = 0xFF;
        }

        while (input[i] != ' ' && input[i] != '\n') {    // sudah selesai membaca perintah
            command[j] = input[i];
            j++;
            i++;
        }
        command[j] = '\0';

        int argcount = 0;   // jumlah argument count
        while (input[i] != '\n') {      // membaca argumen jika ada
            i++;            // mengabaikan space
            j = 0;
            while (input[i] != ' ' && input[i] != '\n') {
                argv[argcount][j] = input[i];
                j++;
                i++;
            }
            argv[argcount][j] = '\0';
            argcount++;
        }
    } else {                    // hanya input enter
        printString("\n");
    }

    argc = argcount;
    

    if (command == "cd") {      // pindah ke folder
        // belum ada implementasi
    } else {                    // 
        interrupt(0x21, 0x20, curdir, argc, argv);                          // taruh argumen
        int result;
        interrupt(0x21, (curdir << 8) | 0x06, command, 0x200, &result);     // executeProgram
    }
}