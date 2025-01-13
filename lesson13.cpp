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
	Expr int_part = multi_func2(x, y)[0];
	Expr flt_part = multi_func2(x, y)[1];

	Func consumer;
	consumer(x, y) = { int_part + 10, flt_part + 10.0f };

	// Tuple reductions.
	{
		Func input_func;
		input_func(x) = sin(x);
		Buffer<float> input = input_func.realize({ 100 });

		Func arg_max;
		arg_max() = { 0, input(0) };

		RDom r(1, 99);
		Expr old_index = arg_max()[0];
		Expr old_max = arg_max()[1];
		Expr new_index = select(old_max < input(r), r, old_index);
		Expr new_max = max(input(r), old_max);
		arg_max() = { new_index, new_max };

		Realization res = arg_max.realize();
		Buffer<int> r0 = res[0];
		Buffer<float> r1 = res[1];

		printf("r0 %d r1 %f\n", r0(0), r1(0));

		// for (int i = 0; i < 99; i++) {
		// 	printf("%d %f\n", i, input(i));
		// }
		// The equivalent C++ is:
		int arg_max_0 = 0;
		float arg_max_1 = input(0);
		for (int r = 1; r < 100; r++) {
			int old_index = arg_max_0;
			float old_max = arg_max_1;
			int new_index = old_max < input(r) ? r : old_index;
			float new_max = std::max(input(r), old_max);
			// In a tuple update definition, all loads and computation
			// are done before any stores, so that all Tuple elements
			// are updated atomically with respect to recursive calls
			// to the same Func.
			arg_max_0 = new_index;
			arg_max_1 = new_max;
		}
		assert(arg_max_0 == r0(0));
		assert(arg_max_1 == r1(0));
	}

	printf("Done\n");
}
