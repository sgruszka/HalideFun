#include <Halide.h>
#include <stdio.h>

using namespace Halide;
int main(int argc, char *argv[])
{
	Func gradient;
	Var x("x"), y("y");
	gradient(x,y ) = x + y;
	Buffer<int> out = gradient.realize({8, 8});
	gradient.compile_to_lowered_stmt("gradient.stmt.html", {}, HTML);
	printf("Success!\n");
}
