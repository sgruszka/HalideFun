#include <Halide.h>
#include <climits>
#include <stdio.h>

#ifdef __SSE2__
#include <emmintrin.h>
#endif

// #include "clock.h"
#include "halide_image_io.h"

using namespace Halide;

#define C_EVALUATION 1

template<typename T, int N>
void compare_results_1d(Buffer<T> halide_result, T (&c_result)[N])
{
	for (int x = 0; x < N; x++) {
		if (halide_result(x) != c_result[x]) {
			printf("halide_result(%d) = %d instead of %d\n",
			       x, halide_result(x), c_result[x]);
			exit(-1);
		}
	}
}

template<typename T, int W, int H>
void compare_results_2d(Buffer<T> halide_result, T (&c_result)[H][W])
{
	for (int y = 0; y < H; y++) {
		for (int x = 0; x < W; x++) {
			if (halide_result(x, y) != c_result[y][x]) {
				printf("halide_result(%d, %d) = %d instead of %d\n",
				       x, y, halide_result(x), c_result[y][x]);
				exit(-1);
			}
		}
	}
}

template<typename T>
void compare_results_buffer(Buffer<T> halide_result, Buffer<T> c_result, int w, int h)
{
	for (int y = 0; y < h; y++) {
		for (int x = 0; x < w; x++) {
			if (halide_result(x, y) != c_result(x, y)) {
				printf("halide_result(%d, %d) = %d instead of %d\n",
				       x, y, halide_result(x), c_result(x, y));
				exit(-1);
			}
		}
	}
}

/*
void scale()  {
		int b_min = INT_MAX;
		int b_max = INT_MIN;
		for (int y = 0; y < in.height(); y++) {
			for (int x = 0; x < in.width(); x++) {
				if (halide_result(x, y) < b_min)
					b_min = halide_result(x,y);
				if (halide_result(x, y) > b_max)
					b_max = halide_result(x,y);
			}
		}
		printf("MAX %d MIN %d\n", b_max, b_min);
		// max_val(0) = maximum(spread(0 + all.x, 0 + all.y));
		// Buffer<int> b_max = max_val.realize({1});
		Func out;
		float factor = 255.0/(b_max - b_min);
		out(x,y) = cast<uint8_t>(factor*(halide_result(x,y) - b_min));
		Buffer<uint8_t> out_buf = out.realize({in.width(), in.height()});
		Tools::save_image(out_buf, "out.png");
}
*/

int main(int argc, char *argv[])
{
	printf("%s\n", __func__);
	Var x("x"), y("y");

	if (0) {
		Func f("f");
		f(x, y) = x + y;

		f(3, 7) = 42;
		f(x, y) = f(x, y) + 17;

		f(x, 3) = f(x, 0) * f(x, 10);
		f(0, y) = f(0, y) / f(3, y);

		f.realize({ 100, 101 });

		Func g("g");
		g(x, y) = x + y;
		g(2, 1) = 8;
		g(x, 0) = g(x, 1);
		g.trace_loads();
		g.trace_stores();
		g.realize({ 4, 4 });
	}

	if (0) {
		Func f("f");
		f(x, y) = (x + y) / 100.0f;
		RDom r(0, 50);
		f(x, r) = f(x, r) * f(x, r);
		Buffer<float> res = f.realize({ 100, 100 });
	}

	if (0) { // Histogram
		Buffer<uint8_t> in_buf = Tools::load_image("images/gray.png");
		Func hist("hist");
		RDom r(0, in_buf.width(), 0, in_buf.height());
		hist(x) = 0;
		hist(in_buf(r.x, r.y)) += 1;

		Buffer<int> res = hist.realize({ 256 });
	}
	// Schedule update steps
	if (0) {
		Func f("f");
		f(x, y) = x * y;
		f(x, 0) = f(x, 8);
		f(0, y) = f(8, y) + 2;

		f.vectorize(x, 4).parallel(y);
		f.update(0).vectorize(x, 4);

		Var yo("yo"), yi("yi");
		f.update(1).split(y, yo, yi, 4).parallel(yo);

		Buffer<int> out = f.realize({ 16, 16 });
#if 1 //ifdef C_EVALUATION
		int res[16][16];

		for (int y = 0; y < 16; y++) { // parallel
			for (int xv = 0; xv < 4; xv++) {
				int x = xv * 4;
				int vec[] = { x, x + 1, x + 2, x + 3 };

				res[y][x] = vec[0] * y;
				res[y][x + 1] = vec[1] * y;
				res[y][x + 2] = vec[2] * y;
				res[y][x + 3] = vec[3] * y;
			}
		}

		for (int xv = 0; xv < 4; xv++) {
			int x = xv * 4;
			int vec[] = { x, x + 1, x + 2, x + 3 };

			res[0][vec[0]] = res[8][vec[0]];
			res[0][vec[1]] = res[8][vec[1]];
			res[0][vec[2]] = res[8][vec[2]];
			res[0][vec[3]] = res[8][vec[3]];
		}

		for (int yo = 0; yo < 4; yo++) { // parallel
			for (int yi = 0; yi < 4; yi++) {
				int y = yo * 4 + yi;
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
	// Reduction (update with function itself as input) in producer-consumer on producer
	if (0) {
		Func consumer("consumer"), producer("producer");
		producer(x) = x * 2;
		producer(x) += 10;
		consumer(x) = 2 * producer(x);

		// producer.trace_stores();
		// producer.trace_loads();
		// consumer.trace_stores();
		// consumer.trace_loads();

		producer.compute_at(consumer, x);

		Buffer<int> halide_result = consumer.realize({ 10 });
#ifdef C_EVALUATION
		int c_result[10];
		for (int x = 0; x < 10; x++) {
			int producer_storage[1];

			producer_storage[0] = x * 2;
			producer_storage[0] += 10; // update step for producer

			c_result[x] = 2 * producer_storage[0];
		}

		for (int x = 0; x < 10; x++) {
			if (halide_result(x) != c_result[x]) {
				printf("halide_result(%d) = %d instead of %d\n",
				       x, halide_result(x), c_result[x]);
				return -1;
			}
		}
#endif
	}
	// Reduction in producer-consumer on consumer
	if (1) {
		Func consumer("consumer"), producer("producer");
		producer(x) = x * 17;
		consumer(x) = 2 * producer(x);
		consumer(x) += 50;

		// producer.trace_stores();
		// producer.trace_loads();
		// consumer.trace_stores();
		// consumer.trace_loads();

		producer.compute_at(consumer, x);

		Buffer<int> halide_result = consumer.realize({ 10 });
#ifdef C_EVALUATION
		int c_result[10];
		for (int x = 0; x < 10; x++) {
			int producer_storage[1];
			producer_storage[0] = x * 17;

			c_result[x] = 2 * producer_storage[0];
		}
		for (int x = 0; x < 10; x++) {
			c_result[x] += 50;
		}

		compare_results_1d(halide_result, c_result);
#endif
	}

	// The consumer references the producer in the update step only
	if (0) {
		Func producer("producer"), consumer("consumer");
		producer(x) = x * 17;
		consumer(x) = 100 - x * 10;
		consumer(x) += producer(x);

		producer.compute_at(consumer, x);

		Buffer<int> halide_result = consumer.realize({ 10 });
#ifdef C_EVALUATION
		int c_result[10];
		for (int x = 0; x < 10; x++) {
			c_result[x] = 100 - x * 10;
		}
		for (int x = 0; x < 10; x++) {
			int producer_storage[1];
			producer_storage[0] = x * 17;
			c_result[x] += producer_storage[0];
		}

		compare_results_1d(halide_result, c_result);
#endif
	}

	// Case 3: The consumer references the producer in multiple steps that share common variables
	if (0) {
		Func producer("producer"), consumer("consumer");
		producer(x) = x * 17;
		consumer(x) = 170 - producer(x);
		consumer(x) += producer(x) / 2;

		producer.store_root().compute_at(consumer, x);

		producer.trace_stores();
		producer.trace_loads();
		consumer.trace_stores();
		consumer.trace_loads();

		Buffer<int> halide_result = consumer.realize({ 5 });

#ifdef C_EVALUATION
		int c_result[5];
		int producer_storage[5];
		for (int x = 0; x < 5; x++) {
			int producer_storage[1];
			producer_storage[x] = x * 17;
			c_result[x] = 170 - producer_storage[x];
		}
		for (int x = 0; x < 5; x++) {
			c_result[x] += producer_storage[x] / 2;
		}
		compare_results_1d(halide_result, c_result);
#endif
	}

	if (0) {
		// Case 4: The consumer references the producer in
		// multiple steps that do not share common variables
		Func producer("producer"), consumer("consumer");
		producer(x, y) = (x * y) / 10 + 8;
		// consumer(x, y) = x + y;
		// consumer(x, 0) += producer(x, x);
		// consumer(0, y) += producer(y, 9 - y);

		// Attempt 2:
		Func producer_1, producer_2, consumer_2;
		producer_1(x, y) = producer(x, y);
		producer_2(x, y) = producer(x, y);

		consumer_2(x, y) = x + y;
		consumer_2(x, 0) += producer_1(x, x);
		consumer_2(0, y) += producer_2(y, 9 - y);

		// The wrapper functions give us two separate handles on
		// the producer, so we can schedule them differently.
		producer_1.compute_at(consumer_2, x);
		producer_2.compute_at(consumer_2, y);

		Buffer<int> halide_result = consumer_2.realize({ 10, 10 });
#ifdef C_EVALUATION
		int c_result[10][10];

		for (int x = 0; x < 10; x++) {
			for (int y = 0; y < 10; y++) {
				c_result[y][x] = x + y;
			}
		}

		for (int x = 0; x < 10; x++) {
			int producer_1_storage[1][1];
			producer_1_storage[0][0] = (x * x) / 10 + 8;
			c_result[0][x] += producer_1_storage[0][0];
		}

		for (int y = 0; y < 10; y++) {
			int producer_2_storage[1][1];
			producer_2_storage[0][0] = (y * (9 - y)) / 10 + 8;
			c_result[y][0] += producer_2_storage[0][0];
		}
		compare_results_2d(halide_result, c_result);
#endif
	}

	if (0) {
		// Case 5: Scheduling a producer under a reduction domain
		// variable of the consumer.
		Func producer("producer"), consumer("consumer");
		RDom r(0, 5);

		producer(x) = x % 8;
		consumer(x) = x + 10;
		consumer(x) += r + producer(x + r);

		producer.compute_at(consumer, r);

		Buffer<int> halide_result = consumer.realize({ 10 });
#ifdef C_EVALUATION
		int c_result[10];

		for (int x = 0; x < 10; x++) {
			c_result[x] = x + 10;
		}

		for (int x = 0; x < 10; x++) {
			for (int r = 0; r < 5; r++) {
				int producer_storage[1];
				producer_storage[0] = (x + r) % 8;
				c_result[x] += r + producer_storage[0];
			}
		}

		compare_results_1d(halide_result, c_result);

#endif
	}

	if (1) {
		// A real-world example of a reduction inside a producer-consumer chain
		Buffer<uint8_t> in_buf = Tools::load_image("images/gray.png");
		Func claped = BoundaryConditions::repeat_edge(in_buf);

		// 5x5 Box , started at (-2, -2)
		RDom r(-2, 5, -2, 5);

		Func local_sum;
		local_sum(x, y) = 0;
		local_sum(x, y) += claped(x + r.x, y + r.y);

		Func blurry;
		blurry(x, y) = cast<uint8_t>(local_sum(x, y) / 25);

		Buffer<uint8_t> halide_result = blurry.realize({ in_buf.width(), in_buf.height() });
#ifdef C_EVALUATION
		Buffer<uint8_t> c_result(in_buf.width(), in_buf.height());

		for (int y = 0; y < in_buf.height(); y++) {
			for (int x = 0; x < in_buf.width(); x++) {
				int local_sum[1];

				local_sum[0] = 0;
				for (int ry = -2; ry < -2 + 5; ry++) {
					for (int rx = -2; rx < -2 + 5; rx++) {
						int clamped_x = std::min(std::max(x + rx, 0), in_buf.width() - 1);
						int clamped_y = std::min(std::max(y + ry, 0), in_buf.height() - 1);
						local_sum[0] += in_buf(clamped_x, clamped_y);
					}
				}

				c_result(x, y) = (uint8_t)(local_sum[0] / 25);
			}
		}

		compare_results_buffer(halide_result, c_result, in_buf.width(), in_buf.height());
#endif
	}

	if (1) {
		// Reduction helpers.
		RDom r(0, 100);

		Func f1;
		f1(x) = sum(x + r) * 7;

		Func f2;
		Func anon;

		anon(x) = 0;
		anon(x) += r + x;
		f2(x) = anon(x) * 7;

		Buffer<int> halide_result1 = f1.realize({ 10 });
		Buffer<int> halide_result2 = f2.realize({ 10 });
#ifdef C_EVALUATION
		int c_result[10];

		for (int x = 0; x < 10; x++) {
			int anon[1];
			anon[0] = 0;
			for (int r = 0; r < 100; r++) {
				anon[0] += r + x;
			}

			c_result[x] = anon[0] * 7;
		}

		compare_results_1d(halide_result1, c_result);
		compare_results_1d(halide_result2, c_result);
#endif
	}

	if (1) {
		// A complex example that uses reduction helpers.
		Buffer<uint8_t> in = Tools::load_image("images/gray.png");
		Func clamped;
		Expr x_clamped = clamp(x, 0, in.width() - 1);
		Expr y_clamped = clamp(y, 0, in.height() - 1);

		clamped(x, y) = in(x_clamped, y_clamped);

		RDom box(-2, 5, -2, 5);
		// Compute the local maximum minus the local minimum
		Func spread;
		spread(x, y) = (maximum(clamped(x + box.x, y + box.y)) -
				minimum(clamped(x + box.x, y + box.y)));

		// Compute the result in strips of 32 scanlines
		Var yo, yi;

		spread.split(y, yo, yi, 32).parallel(yo);
		spread.vectorize(x, 16);

		clamped.store_at(spread, yo).compute_at(spread, yi);
		Buffer<uint8_t> halide_result = spread.realize({ in.width(), in.height() });

#ifdef __SSE2__
		printf("I have sse2\n");
#ifdef _OPENMP
		printf("I have openmp\n");
		double t1 = current_time();
#endif

#ifdef C_EVALUATION
		int W = in.width();
		int H = in.height();
		Buffer<uint8_t> c_result(W, H);

		for (int yo = 0; yo < (H + 31) / 32; yo++) {
			int y_base = std::min(yo * 32, H - 32);

			int clamped_width = (W + 4);
			uint8_t clamped_storage[clamped_width * 8];

			for (int yi = 0; yi < 32; yi++) {
				int y = y_base + yi;

				uint8_t *output_row = &c_result(0, y);

				int min_y_clamped = (yi == 0) ? y - 2 : y + 2;
				int max_y_clamped = y + 2;

				for (int cy = min_y_clamped; cy <= max_y_clamped; cy++) {
					uint8_t *clamped_row = clamped_storage + (cy & 7) * clamped_width;
					int clamped_y = std::min(std::max(cy, 0), H - 1);
					uint8_t *input_row = &in(0, clamped_y);

					// Fill it in with the padding.
					for (int x = -2; x < W + 2; x++) {
						int clamped_x = std::min(std::max(x, 0), W - 1);
						*clamped_row++ = input_row[clamped_x];
					}
				}

				for (int x_vec = 0; x_vec < (W + 15) / 16; x_vec++) {
					int x_base = std::min(x_vec * 16, W - 16);

					__m128i minimum_storage, maximum_storage;

					maximum_storage = _mm_setzero_si128();

					for (int max_y = y - 2; max_y <= y + 2; max_y++) {
						uint8_t *clamped_row = clamped_storage + (max_y & 7) * clamped_width;
						for (int max_x = x_base - 2; max_x <= x_base + 2; max_x++) {
							__m128i v = _mm_loadu_si128((__m128i const *)(clamped_row + max_x + 2));
							maximum_storage = _mm_max_epu8(maximum_storage, v);
						}
					}

					minimum_storage = _mm_cmpeq_epi32(_mm_setzero_si128(), _mm_setzero_si128());

					for (int min_y = y - 2; min_y <= y + 2; min_y++) {
						uint8_t *clamped_row = clamped_storage + (min_y & 7) * clamped_width;
						for (int min_x = x_base - 2; min_x <= x_base + 2; min_x++) {
							__m128i v = _mm_loadu_si128((__m128i const *)(clamped_row + min_x + 2));
							minimum_storage = _mm_min_epu8(minimum_storage, v);
						}
					}

					__m128i spread = _mm_sub_epi8(maximum_storage, minimum_storage);

					_mm_storeu_si128((__m128i *)(output_row + x_base), spread);
				}
			}
		}
#endif
#endif
		compare_results_buffer(halide_result, c_result, W, H);
	}
}
