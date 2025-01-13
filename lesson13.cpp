#include <stdio.h>

#include "Halide.h"

using namespace Halide;

int main(int argc, char *argv[])
{
	Func single_value;
	Var x, y;
	single_value(x, y) = x + y;

	Func color_img;
	Var c;
	color_img(x, y, c) = select(c == 0, 245, c == 1, 42, 132);
	// color_img(x, y, c) = mux(c, { 245, 42, 132 });

	Func brighter;
	brighter(x, y, c) = color_img(x, y, c) + 10;

	Func func_arr[3];
	func_arr[0](x, y) = x + y;
	func_arr[1](x, y) = sin(x);
	func_arr[2](x, y) = cos(x);

	Func multi_func;
	multi_func(x, y) = Tuple(x + y, sin(x * y));
	{
		Realization r = multi_func.realize({ 80, 60 });
		assert(r.size() == 2);
		Buffer<int> r0 = r[0];
		Buffer<float> r1 = r[1];
		assert(r0(30, 40) = 30 + 40);
		assert(r1(30, 40) = sin(30 * 40));
	}
	{
		int multi_valued0[60][80];
		float multi_valued1[60][80];
		for (int y = 0; y < 60; y++) {
			for (int x = 0; x < 80; x++) {
				multi_valued0[y][x] = x + y;
				multi_valued1[y][x] = sinf(x * y);
			}
		}
		assert(multi_valued0[30][40] == 40 + 30);
		assert(multi_valued1[30][40] == sinf(40 * 30));
	}

	Func multi_func2;
	multi_func2(x, y) = { x + y, sin(x * y) };
	multi_func2.realize({ 5, 10 });
	printf("Done\n");
}
