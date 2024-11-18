// vim: set tabstop=1 shiftwidth=1 :
// test file 1
#include <cstdio> //line starting with whitespace -> '#' is preprocessor directive and will be ignored
#include <cstdlib>
// inside-loop=0

class C;

class A {
public:
				void f(void) {
								continue;
								for (;;) {
												continue;
								}
				}
				class B : C {
				private:
								int *x(int *a[], int b, ...) {
												do continue; while (0);
												continue;
								}
				}
};

int main(int argc, char *argv[], ...) {
	for (int i = 0; i < 10; i++) /*inside-loop=1*/continue/*ok*/; /*inside-loop=0*/// should not be triggered
	do {/*inside-loop=1*/int i =0; continue/*ok*/;} /*inside-loop=0*/while (0); // should not be triggered
	int i = 0;
	//for (;;) (1, continue);
	while (i++ < 3) {/*inside-loop=1*/
		continue/*ok*/; // should not be triggered
	}
	/*vvv inside-comment=1; ok*/
	// continue should not be triggered
/*inside-comment=0*/
	/*vvv inside-string=1; ok*/
	"continue"/*inside-string=0*/; // should not be triggered
	continue/*inside-loop=0 inside-comment=0 inside-string=0 => ERROR*/; // should be triggered
	continueaaa; // should not be triggered because it is a different token
	return 0;
}

namespace abc {
	void d(void) {
		continue;
		for (auto a : b) {
			continue;
		}
	}
	// for () {
	// 	continue;
	// }
}

// inside-loop=0
continue/*inside-loop=0 inside-comment=0 inside-string=0 => ERROR*/; // should be triggered

for () {//inside-loop=1 but isnide-func=0 but I cannot actually detect a func
	// unless
	// count brackets??? ignore namespace and class???
	// not for today
	continue/*inside-loop=1 => OK for now*/; // should be triggered but cannot be actually caught
	// (reason: for outside function is not valid but is not checked in current impl)
}//inside-loop=0

