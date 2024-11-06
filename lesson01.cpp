#include </home/stasiu/halide-install/include/Halide.h>
#include <stdio.h>

int main(int argc, char **argv) 
{
	Halide::Func gradient;
	Halide::Var x, y;
	Halide::Expr e = x + y;

	gradient(x, y) = e;

	Halide::Buffer<int32_t> output = gradient.realize({800,600});

	for (int j = 0; j < output.height(); j++) {
		for (int i = 0; i < output.width(); i++) {
			if (output(i, j) != i + j) {
				printf("Something went wrong\n"
					"Pixel (%d, %d), suppose be %d, but is %d\n",
					i, j, i+j, output(i, j));
				return -1;
			}
		}
	}

	printf("SUCCESS\n");
	return 0;

}
