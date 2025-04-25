
#include <Halide.h>
#include <stdio.h>

#include "halide_image_io.h"

using namespace Halide;

int main(int argc, char *argv[])
{
	constexpr int W = 8;
	constexpr int H = 12;

	Var x("x"), y("y"), fused("fused");
	Func g("g");
	g(x, y) = cast<int>(x + y);

	Buffer<int> out_g = g.realize({ W, H });

	Func f1("f"), in("in");
	in(x, y) = out_g(x, y);
	f1(x, y) = cast<uint8_t>(min((cast<int>(in(x, y)) + 10), 255));
	f1.trace_stores();

	RDom rd(0, W, 0, H);
	Func sum("sum");
	sum() = cast<uint64_t>(0);
	sum() += cast<uint64_t>(in(rd.x, rd.y));

	Func f2("f2");

	f2(x, y) = cast<uint8_t>(min(in(x, y) * 2.0f, 255));
	f2.trace_stores();

	// sum.compute_with(in, x);
	// g.print_loop_nest();
	//f2.compute_root();
	//f1.compute_with(f2, fused, LoopAlignStrategy::AlignStart);
	//
	Func f12("f12");
	f12(x, y) = Tuple(f1(x, y), f2(x, y), sum());
	f12.trace_stores();

	Realization r12 = f12.realize({ W, H });

	f12.print_loop_nest();
	//	sum.print_loop_nest();

	// sum.trace_realizations();
	// f12.trace_realizations();

	// Buffer<uint8_t> buf = f1.realize({ W, H });
	// Buffer<uint8_t> buf1 = f2.realize({ W, H });
	Buffer<uint8_t> buf1 = r12[0];
	Buffer<uint8_t> buf2 = r12[1];

	//	Buffer<uint64_t> sum_buf = sum.realize();

	//	printf("sum_buf %llu\n", (unsigned long long)sum_buf(0));

	printf("%u %u\n", buf1(0, 1), buf2(3, 8));
}
