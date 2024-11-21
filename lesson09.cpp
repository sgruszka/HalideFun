#include <Halide.h>
#include <stdio.h>

#ifdef __SSE2__
#include <emmintrin.h>
#endif

// #include "clock.h"
#include "halide_image_io.h"

using namespace Halide;

int main(int argc, char *argv[]) 
{
	printf("%s\n", __func__);
	Var x("x"), y("y");

	Buffer<uint8_t> in_buf = Tools::load_image("images/gray.png");
	{
		Func f("f");
		f(x,y) = x + y;

		f(3, 7) = 42;
		f(x,y) = f(x,y) + 17;

		f(x, 3) = f(x, 0) * f(x, 10);
		f(0, y) = f(0, y) / f(3, y);

		f.realize({100, 101});

		Func g("g");
		g(x, y) = x + y;
		g(2, 1) = 8;
		g(x, 0) = g(x, 1);
		g.trace_loads();
		g.trace_stores();
		g.realize({4, 4});
	}

	{
		Func f("f");
		f(x,y) = (x+y) / 100.0f;
		RDom r(0, 50);
		f(x, r) = f(x, r) * f(x, r);
		Buffer<float> res = f.realize({100, 100});
	}

	{
		Func hist("hist");
		RDom r(0, in_buf.width(), 0, in_buf.height());
		hist(in_buf(r.x, r.y)) += 1;

		Buffer<int> res = hist.realize({256});
	} 
	{
		Func f("f");
		f(x,y) = x*y;
		f(x,0) = f(x,8);
		f(0,y) = f(8,y) + 2;

		f.vectorize(x, 4).parallel(y);
		f.update(0).vectorize(x, 4);

		Var yo("yo"), yi("yi");
		f.update(1).split(y, yo, yi, 4).parallel(yo);
		
		Buffer<int> out = f.realize({16, 16});
	}



}
