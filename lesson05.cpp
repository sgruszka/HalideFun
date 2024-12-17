#include <Halide.h>
#include <stdio.h>

#include "halide_image_io.h"

using namespace Halide;

int main(int argc, char *argv[])
{
	Var x("x"), y("y");
	constexpr int N = 4;

	if (0) {
		Func gradient("gradient row");
		gradient(x, y) = x + y;
		gradient.trace_stores();

		Buffer<int> out = gradient.realize({ N, N });

		printf("Code for schedule:");
		gradient.print_loop_nest();
		printf("\n");

		printf("C code:\n");
		for (int y = 0; y < N; y++) {
			for (int x = 0; x < N; x++) {
				printf("Evaluate (%d, %d) = %d", x, y, x + y);
			}
		}
		printf("\n\n");
	}
	if (0) {
		Func gradient("gradient col");
		gradient(x, y) = x + y;
		gradient.trace_stores();

		gradient.reorder(y, x);

		Buffer<int> out = gradient.realize({ N, N });

		printf("Code for schedule:");
		gradient.print_loop_nest();
		printf("\n");

		printf("C code:\n");
		for (int x = 0; x < N; x++) {
			for (int y = 0; y < N; y++) {
				printf("Evaluate (%d, %d) = %d", x, y, x + y);
			}
		}
		printf("\n\n");
	}
	if (0) {
		Func gradient("gradient_split");
		gradient(x, y) = x + y;
		gradient.trace_stores();

		Var x_outer, x_inner;
		gradient.split(x, x_outer, x_inner, 2);

		Buffer<int> out = gradient.realize({ 8, 3 });

		printf("Code for schedule:");
		gradient.print_loop_nest();
		printf("\n");

		printf("C code:\n");
		for (int y = 0; y < 3; y++) {
			for (int x_outer = 0; x_outer < 8 / 2; x_outer++) {
				for (int x_inner = 0; x_inner < 2; x_inner++) {
					int x = x_outer * 2 + x_inner;
					printf("Evaluate (%d, %d) = %d\n", x, y, x + y);
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
		Buffer<int> out = gradient.realize({ N, N });
		printf("Code for schedule:");
		gradient.print_loop_nest();
		printf("\n");

		printf("C code:\n");
		for (int fused = 0; fused < N * N; fused++) {
			int y = fused / N;
			int x = fused % N;
			printf("Evaluate (%d, %d) = %d\n", x, y, x + y);
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

		Buffer<int> out = gradient.realize({ 2 * N, 2 * N });
		printf("Code for schedule:");
		gradient.print_loop_nest();
		printf("\n");

		printf("C code:\n");

		for (int y_outer = 0; y_outer < N / 2; y_outer++) {
			for (int x_outer = 0; x_outer < N / 2; x_outer++) {
				for (int y_inner = 0; y_inner < N; y_inner++) {
					for (int x_inner = 0; x_inner < N; x_inner++) {
						int x = x_outer * 2 + x_inner;
						int y = y_outer * 2 + y_inner;
						printf("Evaluate (%d, %d) = %d\n", x, y, x + y);
					}
				}
			}
		}
		printf("\n\n");
	}

	if (0) {
		Func gradient("gradient_in_vectors");
		gradient(x, y) = x + y;
		gradient.trace_stores();

		Var x_inner, x_outer;
		gradient.split(x, x, x_inner, N);
		gradient.vectorize(x_inner);

		Buffer<int> out = gradient.realize({ 2 * N, N });
		printf("Code for schedule:");
		gradient.print_loop_nest();
		printf("\n");

		printf("C code:\n");
		for (int y = 0; y < N; y++) {
			for (int x = 0; x < N / 2; x++) {
				int x_vec[] = { x * N + 0, x * N + 1, x * N + 2, x * N + 3 };
				int val[] = { x_vec[0] + y, x_vec[1] + y, x_vec[2] + y, x_vec[3] + y };

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
		gradient.split(x, x_outer, x_inner, N / 2);
		gradient.unroll(x_inner);

		Buffer<int> out = gradient.realize({ N, N });
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

		Buffer<int> out = gradient.realize({ 7, 2 });
		printf("Code for schedule:");
		gradient.print_loop_nest();
		printf("\n");

		printf("C code:\n");
		for (int y = 0; y < 2; y++) {
			int x_outer_factor = 7 / 3 + (7 % 3 != 0 ? 1 : 0);
			for (int x_outer = 0; x_outer < x_outer_factor; x_outer++) {
				for (int x_inner = 0; x_inner < 3; x_inner++) {
					int x = x_outer * x_outer_factor;
					if (x > 7 - 3)
						x = 7 - 3;
					x += x_inner;
					printf("Evaluate (%d, %d) = %d\n", x, y, x + y);
				}
			}
		}
		printf("\n\n");
	}
	if (0) {
		Func gradient("gradient_fused_tilled");
		gradient(x, y) = x + y;
		gradient.trace_stores();

		Var x_inner, x_outer, y_inner, y_outer, tile_index;
		gradient.split(x, x_outer, x_inner, 4);
		gradient.split(y, y_outer, y_inner, 4);
		gradient.reorder(x_inner, y_inner, x_outer, y_outer);
		gradient.fuse(x_outer, y_outer, tile_index);
		gradient.parallel(tile_index);

		Buffer<int> out = gradient.realize({ 8, 8 });
		printf("Code for schedule:");
		gradient.print_loop_nest();
		printf("\n");
	}

	if (1) {
		Func g("gradient fast");
		g(x, y) = x + y;
		g.trace_stores();

		Var x_inner, x_outer, y_inner, y_outer, tile_index;
		g
			.tile(x, y, x_outer, y_outer, x_inner, y_inner, 64, 64)
			.fuse(x_outer, y_outer, tile_index)
			.parallel(tile_index);

		Var x_inner_outer, y_inner_outer, x_vectors, y_pairs;
		g
			.tile(x_inner, y_inner, x_inner_outer, y_inner_outer, x_vectors, y_pairs, 4, 2)
			.vectorize(x_vectors)
			.unroll(y_pairs);

		Buffer<int> out = g.realize({ 350, 250 });
		printf("Code for schedule:");
		g.print_loop_nest();
		printf("\n");

		constexpr float max_val = 350.0f + 250.0f;
		constexpr float scale_factor = 255.0 / max_val;
		Func scale;
		Expr val = out(x, y);

		scale(x, y) = cast<uint8_t>(min(val * scale_factor, 255));
		Buffer<uint8_t> im_out = scale.realize({ out.width(), out.height() });

		Tools::save_image(im_out, "gradient.png");
	}
}
