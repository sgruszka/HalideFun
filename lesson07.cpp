#include <Halide.h>
#include <stdio.h>
#include "halide_image_io.h"

using namespace Halide;
using namespace Halide::Tools;

int main(int argc, char *argv[])
{
	Var x("x"), y("y"), c("c");

	{
		Buffer<uint8_t> in_buf = load_image("images/rgb.png");
		Func in("in");
		Func blur_x("blur_x"), blur_y("blur_y");

		const int w = in_buf.width();
		const int h = in_buf.height();
		const int nc = in_buf.channels();
		printf("Sizes %d %d %d\n", w, h, nc);

		in(x,y,c) = cast<uint16_t>(in_buf(x,y,c));

		blur_x(x,y,c) = (in(x-2,y,c) + in(x-1,y,c) + 2*in(x,y,c) + in(x+1,y,c)+in(x+2,y,c)) / 6;
		blur_y(x,y,c) = (blur_x(x,y-2,c) + blur_x(x,y-1,c) + 2*blur_x(x,y,c) + blur_x(x,y+1,c)+blur_x(x,y+2,c)) / 6;

		Func out;

		out(x,y,c) = cast<uint8_t>(blur_y(x,y,c));

		Buffer<uint8_t> out_buf(w - 4, h - 4, nc);
		out_buf.set_min(2,2);
		out.realize(out_buf);

		save_image(out_buf, "blur.png");
	}

	{
		Buffer<uint8_t> in_buf = load_image("images/rgb.png");
		const int w = in_buf.width();
		const int h = in_buf.height();
		const int nc = in_buf.channels();
		printf("Sizes %d %d %d\n", w, h, nc);

		Func clamped("clamped");

		Expr clamped_x = clamp(x, 0, in_buf.width() - 1);
		Expr clamped_y = clamp(y, 0, in_buf.height() - 1);

		clamped(x,y,c) = in_buf(clamped_x, clamped_y, c);

		Func in("input16"), blur_x("blur_x"), blur_y("blur_y");

		in(x,y,c) = cast<uint16_t>(clamped(x,y,c));

		blur_x(x,y,c) = (in(x-2,y,c) + in(x-1,y,c) + 2*in(x,y,c) + in(x+1,y,c)+in(x+2,y,c)) / 6;
		blur_y(x,y,c) = (blur_x(x,y-2,c) + blur_x(x,y-1,c) + 2*blur_x(x,y,c) + blur_x(x,y+1,c)+blur_x(x,y+2,c)) / 6;

		Func out;

		out(x,y,c) = cast<uint8_t>(blur_y(x,y,c));
		Buffer<uint8_t> out_buf = out.realize({w, h, nc});

		save_image(out_buf, "blur2.png");
	}

}
