#include <cstdio>
#include <cstdlib>

int main(void) {
	for (int i = 0; i < 10; i++) {
		if (i == 5) continue;
		printf("%d ", i);
	}
	// continue;
	printf("\n");
	return 0;
}