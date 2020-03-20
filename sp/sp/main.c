#include <stdio.h>
#include <stdlib.h>

int main() {
	int* a = (int*)malloc(sizeof(int) * 8);
	for (int i = 0; i < 8; i++) {
		a[i] = i + 1;
	}
	for (int i = 0; i < 8; i++) {
		printf("%d\n", a[i]);
	}
	
	return 0;
}