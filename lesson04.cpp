#include <Halide.h>
#include <stdio.h>
#include "halide_image_io.h"

using namespace Halide;

int main(int argc, char *argv[])
{
	Var x("x"), y("y");

	Func paraller_gradinet("paraller_gradinet");
	paraller_gradinet(x,y) = x + y;

	// paraller_gradinet.trace_stores();
	paraller_gradinet.parallel(y);
	// printf("Evaluating paraller gradient");

	constexpr int N = 100;
	Buffer<int> out1 = paraller_gradinet.realize({N, N});

	Expr val = out1(x, y);
	float max = N + N;
	float scale = 255 / max;

	Func func2;
	func2(x, y) = Halide::cast<uint8_t>(min(scale * val, 255));
	Buffer<uint8_t> out2_buf = func2.realize({out1.width(), out1.height()});

	printf("\nSaving image!\n");
	Halide::Tools::save_image(out2_buf, "gradient.png");

	printf("\nSuccess!\n");

	{

		Func one;
		Var x, y;

		one(x, y) = sin(x) + print(cos(y), "<- this is cos(", y , ") when  x= ", x);

		printf("\nEvaluating and printing\n");
		one.realize({4, 4});

		Expr e = cos(y);
		e = print_when(x==47 && y == 64, e,  "<- this is cos(", y , ") when  x= ", x,  "and y = ", y);

		Func g;

		g(x, y) = sin(x) + e;
		g.realize({640, 480});
	}



}
