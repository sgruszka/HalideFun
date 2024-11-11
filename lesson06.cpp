#include <Halide.h>
#include <stdio.h>

using namespace Halide;

int main(int argc, char *argv[])
{
	Func g("gradient");
	Var x("x"), y("y");
	g(x, y) = x + y;
	g.trace_stores();

	printf("Evaluating gradient from (0,0) to (7,7)\n");
	Buffer<int> res(8,8);
	g.realize(res);

	for (int y = 0; y < 8; y++) {
		for (int x = 0; x < 8; x++) {
			if (res(x,y) != x+y) {
				printf("What happened?");
				return -1;
			}
		}
	}

	printf("Evaluating gradient from (100,50) to (104,56)\n");
	Buffer<int> shiffted(5,7);
	shiffted.set_min(100, 50);
	g.realize(shiffted);

	for (int y = 50; y < 57; y++) {
		for (int x = 100; x < 105; x++) {
			if (shiffted(x,y) != x+y) {
				printf("What happened with shifted?");
				return -1;
			}
		}
	}

	return 0;
}
