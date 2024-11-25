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

	{ // Histogram
		Func hist("hist");
		RDom r(0, in_buf.width(), 0, in_buf.height());
		hist(x) = 0;
		hist(in_buf(r.x, r.y)) += 1;

		Buffer<int> res = hist.realize({256});
	}
	{ // Schedule update steps
		Func f("f");
		f(x,y) = x*y;
		f(x,0) = f(x,8);
		f(0,y) = f(8,y) + 2;

		f.vectorize(x, 4).parallel(y);
		f.update(0).vectorize(x, 4);

		Var yo("yo"), yi("yi");
		f.update(1).split(y, yo, yi, 4).parallel(yo);

		Buffer<int> out = f.realize({16, 16});
#if 1  //ifdef C_EVALUATION
		int res[16][16];

		for (int y = 0; y < 16; y++) { // parallel
			for (int xv = 0; xv < 4; xv++) {
				int x = xv*4;
				int vec[] = {x, x+1, x+2, x+3};

				res[y][x] = vec[0]*y;
				res[y][x+1] = vec[1]*y;
				res[y][x+2] = vec[2]*y;
				res[y][x+3] = vec[3]*y;
			}
		}

		for (int xv = 0; xv < 4; xv++) {
			int x = xv*4;
			int vec[] = {x, x+1, x+2, x+3};

			res[0][vec[0]] = res[8][vec[0]];
			res[0][vec[1]] = res[8][vec[1]];
			res[0][vec[2]] = res[8][vec[2]];
			res[0][vec[3]] = res[8][vec[3]];
		}

		for (int yo = 0; yo < 4; yo++) { // parallel
			for (int yi = 0; yi < 4; yi++) {
				int y = yo*4 + yi;
				res[y][0] = res[y][8] + 2;
				// printf("(0, %d) = %d\n", y, res[y][0]);
			}
		}

	        // Check the C and Halide results match:
		for (int y = 0; y < 16; y++) {
		    for (int x = 0; x < 16; x++) {
			if (out(x, y) != res[y][x]) {
			    printf("halide_result(%d, %d) = %d instead of %d\n",
				   x, y, out(x, y), res[y][x]);
			    return -1;
			}
		    }
		}
#endif
	}
	{
		Func consumer("consumer"), producer("producer");
		producer(x) = x*17;
		consumer(x) = producer(x)*2;
		consumer(x) += 50;

		producer.trace_stores();
		producer.trace_loads();
		consumer.trace_stores();
		consumer.trace_loads();

		producer.compute_at(consumer, x);
		consumer.realize({10});
	}



}
