#include <Halide.h>
#include <stdio.h>

using namespace Halide;

int main(int argc, char **argv) 
{
	Func brighter;
	Var x, y;
	Param<uint8_t> offset;
	ImageParam input(type_of<uint8_t>(), 2);

	brighter(x, y) = input(x,y) + offset;
	brighter.vectorize(x,16).parallel(y);

	brighter.compile_to_static_library("lesson_10_halide", {input, offset}, "brighter");

	printf("Compiled_to_static_library\n");
	return 0;

}
