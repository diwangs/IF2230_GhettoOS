#ifndef MATH_H
#define MATH_H
// Biar shell dan kernel bisa #include
int mod(int a, int b) { 
	while (a >= b) a -= b;
	return a;
}

int div(int a, int b) {
	int q = 0;
	while (q * b <= a) q += 1;
	return q - 1;
}
#endif