void main() {
    int* res;
	enableInterrupts();
    interrupt(0x21, 0x35, 0, 0, 0);
	interrupt(0x21, 0x07, &res, 0, 0);
}