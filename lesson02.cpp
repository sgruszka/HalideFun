#include <cstdint>
#include <Halide.h>
#include <sys/types.h>
#include "halide_image_io.h"

using namespace Halide::Tools;

int main(int argc, char *argv[]) {

	Halide::Buffer<uint8_t> input = load_image("images/rgb.png");
	Halide::Func output;
	// COPY to OUTPUT not working
	Halide::Var x, y, c;
	Halide::Expr val = input(x, y, c);
	output(x, y, c) = Halide::cast<uint8_t>(min(val*3.0f, 255));

	// output(x,y,c) = input(x,y, c);
	//
	Halide::Buffer<uint8_t> out = output.realize({input.width(), input.height(), input.channels()});

	// brighter(x, y, c) = Halide::cast<uint8_t>(min(input(x, y, c) * 1.5f, 255));
	// Halide::Func brighter;
	// Halide::Var x, y , c;
	// Halide::Expr value = input(x,y,c);
	// value = Halide::cast<float>(value);
	// value = value * 1.5f;
	// value = Halide::min(value, 255.0f);
	// value = Halide::cast<uint8_t>(value);
	//
	// brighter(x, y, c) = value;
	//
	// Halide::Buffer<uint8_t> out = brighter.realize({input.width(), input.height(), input.channels()});
	save_image(out, "out2.png");

	printf("Success!");
	return 0;
}
