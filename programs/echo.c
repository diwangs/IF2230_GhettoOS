void main(){
    char curdir,  res;
    char *argv;
    // interrupt(0x21, 0x0, "Hi", 0, 0);
    interrupt(0x21, 0x21, &curdir, 0, 0);
    interrupt(0x21, 0x23, 0, argv, 0);
    interrupt(0x21, 0x0, argv, 0, 0);
    interrupt(0x21, 0x0, "\r\n", 0, 0);
    interrupt(0x21, (curdir << 8) | 0x06, "shell", 0x2000, &res);
}