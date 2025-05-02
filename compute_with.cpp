
#include <Halide.h>
#include <stdio.h>

// #include "halide_image_io.h"

using namespace Halide;

int main(int argc, char *argv[])
{
	constexpr int W = 2;
	constexpr int H = 3;

	Var x("x"), y("y");
	Func input("input");
	Func g("g"), f("f");

	input(x, y) = x + y;

	f(x, y) = input(x, y);
	f(x, y) += 5;
	g(x, y) = x - y;
	g(x, y) += 10;

	f.compute_with(g, y);
	f.update().compute_with(g.update(), x);

	// f.trace_stores();
	// g.trace_stores();

	Pipeline({ f, g }).print_loop_nest();
	Pipeline({ f, g }).realize({ W, H });
}
