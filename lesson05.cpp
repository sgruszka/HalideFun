#include <Halide.h>
#include <stdio.h>

using namespace Halide;

int main(int argc, char *argv[])
{
	Var x("x"), y("y");
	constexpr int N = 4;

	if (0) {
		Func gradient("gradient row");
		gradient(x,y) = x + y;
		gradient.trace_stores();

		Buffer<int> out = gradient.realize({N, N});

		printf("Code for schedule:");
		gradient.print_loop_nest();
		printf("\n");

		printf("C code:\n");
		for (int y = 0; y < N; y++) {
			for (int x = 0; x < N; x++) {
				printf("Evaluate (%d, %d) = %d", x, y, x+y);
			}
		}
		printf("\n\n");
	}
	if (0) {
		Func gradient("gradient col");
		gradient(x,y) = x + y;
		gradient.trace_stores();

		gradient.reorder(y,x);

		Buffer<int> out = gradient.realize({N, N});

		printf("Code for schedule:");
		gradient.print_loop_nest();
		printf("\n");
		
		printf("C code:\n");
		for (int x = 0; x < N; x++) {
			for (int y = 0; y < N; y++) {
				printf("Evaluate (%d, %d) = %d", x, y, x+y);
			}
		}
		printf("\n\n");
	}
	if (0) {
		Func gradient("gradient_split");
		gradient(x,y) = x + y;
		gradient.trace_stores();
		
		Var x_outer, x_inner;
		gradient.split(x, x_outer, x_inner, N/2);

		Buffer<int> out = gradient.realize({N, N});

		printf("Code for schedule:");
		gradient.print_loop_nest();
		printf("\n");

		printf("C code:\n");
		for (int y = 0; y < N; y++) {
			for (int x_outer = 0; x_outer < N/2; x_outer++) {
				for (int x_inner = 0; x_inner < N/2; x_inner++) {
					int x = x_outer*2 + x_inner;
					printf("Evaluate (%d, %d) = %d\n", x, y, x+y);
				}
			}
		}
		printf("\n\n");
	}
	if (0) {
		Func gradient("gradient_fused");
		gradient(x, y) = x + y;
		gradient.trace_stores();

		Var fused;
		gradient.fuse(x, y, fused);
		Buffer<int> out = gradient.realize({N, N});
		printf("Code for schedule:");
		gradient.print_loop_nest();
		printf("\n");

		printf("C code:\n");
		for (int fused = 0; fused < N*N; fused++) {
			int y = fused / N;
			int x = fused % N;
			printf("Evaluate (%d, %d) = %d\n", x, y, x+y);
		}
		printf("\n\n");
	}

	if (0) {
		Func gradient("gradient_tiled");
		gradient(x, y) = x + y;
		gradient.trace_stores();

		Var x_outer, x_inner, y_outer, y_inner;
		gradient.split(x, x_outer, x_inner, N);
		gradient.split(y, y_outer, y_inner, N);
		gradient.reorder(x_inner, y_inner, x_outer, y_outer);

		Buffer<int> out = gradient.realize({2*N, 2*N});
		printf("Code for schedule:");
		gradient.print_loop_nest();
		printf("\n");

		printf("C code:\n");

		for (int y_outer = 0; y_outer < N; y_outer++) {
			for (int x_outer = 0; x_outer < N; x_outer++) {
				for (int y_inner = 0; y_inner < N; y_inner++) {
					for (int x_inner = 0; x_inner < N; x_inner++) {
						int x = x_outer*2 + x_inner;
						int y = y_outer*2 + y_inner;
						printf("Evaluate (%d, %d) = %d\n", x, y, x+y);
					}
				}
			}
		}
		printf("\n\n");
	}

	if (1) {
		Func gradient("gradient_in_vectors");
		gradient(x, y) = x + y;
		gradient.trace_stores();

		Var x_inner, x_outer;
		gradient.split(x, x, x_inner, N);
		gradient.vectorize(x_inner);

		Buffer<int> out = gradient.realize({2*N, N});
		printf("Code for schedule:");
		gradient.print_loop_nest();
		printf("\n");

		printf("C code:\n");
		for (int y = 0; y < N; y++) {
			for (int x = 0; x < N/2; x++ ) {
				int x_vec[] = {x*N + 0, x*N + 1, x*N + 2, x*N + 3};
				int val[] = {x_vec[0] + y, x_vec[1] + y, x_vec[2] + y, x_vec[3] + y};

				printf("Evaluating at <%d, %d, %d, %d>, <%d, %d, %d, %d> -> <%d, %d, %d, %d>\n",
					x_vec[0], x_vec[1], x_vec[2], x_vec[3], y, y, y, y,
					val[0], val[1], val[2], val[3]);
			}
		}
		printf("\n\n");
	}

	if (0) {
		Func gradient("gradient_unroll");
		gradient(x, y) = x + y;
		gradient.trace_stores();

		Var x_inner, x_outer;
		gradient.split(x, x_outer, x_inner, N/2);
		gradient.unroll(x_inner);

		Buffer<int> out = gradient.realize({N, N});
		printf("Code for schedule:");
		gradient.print_loop_nest();
		printf("\n");
	}

	if (0) {
		Func gradient("gradient_split_7x2");
		gradient(x, y) = x + y;
		gradient.trace_stores();

		Var x_inner, x_outer;
		gradient.split(x, x_outer, x_inner, 3);
		gradient.unroll(x_inner);

		Buffer<int> out = gradient.realize({7, 2});
		printf("Code for schedule:");
		gradient.print_loop_nest();
		printf("\n");
	}
}
