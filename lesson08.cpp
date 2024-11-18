#include <Halide.h>
#include <stdio.h>
#include <cmath>
// #include "halide_image_io.h"
// #

#define C_EVALUATION 1

using namespace Halide;

int main(int argc, char *argv[])
{
	Var x("x"), y("y");
	if (0) {
		Func producer("producer_default"), consumer("consumer_default");
		producer(x,y) = sin(x*y);

		consumer(x,y) = (producer(x,y) + producer(x,y+1) + producer(x+1, y) + producer(x+1,y+1))/4;

		if (1)
			producer.compute_root();

		producer.trace_stores();
		consumer.trace_stores();

		consumer.realize({4, 4});

		consumer.print_loop_nest();

#ifdef C_EVALUATION
		if (1) { // compute root
			float producer_storage[5][5];
			for (int y = 0; y < 5; y++) {
				for (int x = 0; x < 5; x++) {
					producer_storage[y][x] = sinf(x*y);
				}
			}
			for (int y = 0; y < 4; y++) {
				for (int x = 0; x < 4; x++) {
					float res = (	producer_storage[y][x] +
							producer_storage[y+1][x] +
							producer_storage[y][x+1] +
							producer_storage[y+1][x+1]	)/4;
					printf("Evaluate (%d, %d) = %f\n", x, y, res);
				}
			}
		} else {
			for (int y = 0; y < 4; y++) {
				for (int x = 0; x < 4; x++) {
					float consumer = (sinf(x*y) + sinf(x*(y+1)) + sinf((x+1)*y) + sinf((x+1)*(y+1)))/4;
					printf("Evaluate (%d, %d) = %f\n", x, y, consumer);
				}
			}
		}
#endif
	}
	if (0) {
		Func producer("producer_at"), consumer("consumer_at");
		producer(x,y) = sin(x*y);
		consumer(x,y) = (producer(x,y) + producer(x,y+1) + producer(x+1, y) + producer(x+1,y+1))/4;

		producer.trace_stores();
		consumer.trace_stores();

		producer.compute_at(consumer, y);

		consumer.realize({4, 4});
		consumer.print_loop_nest();
#ifdef C_EVALUATION
		for (int y = 0; y < 4; y++) {
			float producer_storage[2][5];
			for (int py = y; py < y+2; py++) {
				for (int px = 0; px < 5; px++) {
					producer_storage[py - y][px] = sin(px*py);
				}
			}

			for (int x = 0; x < 4; x++) {
				float consumer = (producer_storage[0][x] +
						producer_storage[1][x] +
						producer_storage[0][x+1] +
						producer_storage[1][x+1]
						)/4;
				printf("Evaluate (%d, %d) = %f\n", x, y, consumer);
			}
		}

#endif
	}
	if (0) {
		Func producer("producer_root_y"), consumer("consumer_root_y");
		producer(x,y) = sin(x*y);
		consumer(x,y) = (producer(x,y) + producer(x,y+1) + producer(x+1, y) + producer(x+1,y+1))/4;

		producer.store_root();
		producer.compute_at(consumer, y);

		producer.trace_stores();
		consumer.trace_stores();

		consumer.realize({4, 4});
		consumer.print_loop_nest();

#ifdef C_EVALUATION
		float producer_storage[5][5];
		for (int y = 0; y < 4; y++) {

			for (int py = y; py < y + 2; py++) {
				if (y > 0 && py == y)
					continue;

				for (int px = 0; px < 5; px++) {
					producer_storage[py][px] = sinf(py*px);
					printf("Store producer (%d, %d) = %f\n", px, py, producer_storage[py][px]);
				}
			}

			for (int x = 0; x < 4; x++) {
				float consumer = (producer_storage[y][x] +
						producer_storage[y+1][x] +
						producer_storage[y][x+1] +
						producer_storage[y+1][x+1]
						)/4;
				printf("Evaluate consumer (%d, %d) = %f\n", x, y, consumer);
			}
		}

#endif
	}
	if (1) {
		Func producer("producer_root_x"), consumer("consumer_root_x");
		producer(x,y) = sin(x*y);
		consumer(x,y) = (producer(x,y) + producer(x,y+1) + producer(x+1, y) + producer(x+1,y+1))/4;

		producer.store_root().compute_at(consumer, x);

		producer.trace_stores();
		consumer.trace_stores();

		consumer.realize({4, 4});
		consumer.print_loop_nest();
#ifdef C_EVALUATION
		float producer_storage[2][5];

		for (int y = 0; y < 4; y++) {
			for (int x = 0; x < 4; x++) {

				if (y == 0 && x == 0) {
					producer_storage[0][0] = sinf(0);
					printf("Store producer (%d, %d) = %f\n", x, y, producer_storage[0][0]);
				}
				if (y == 0) {
					producer_storage[0][x+1] = sinf(0);
					printf("Store producer (%d, %d) = %f\n", y, x+1, producer_storage[0][x+1]);
				}
				if (x == 0) {
					producer_storage[(y+1) & 1][0] = sinf(0);
					printf("Store producer (%d, %d) = %f\n", y+1, x, producer_storage[0][x+1]);
				}

				producer_storage[(y+1) & 1][x+1] = sinf(x*y);
				printf("Store producer (%d, %d) = %f\n", x+1, y+1 , producer_storage[(y+1)&1][x+1]);

				float consumer = (producer_storage[y & 1][x] +
						producer_storage[(y+1) & 1][x] +
						producer_storage[y & 1][x+1] +
						producer_storage[(y+1) & 1][x+1]
						)/4;
				printf("Evaluate consumer (%d, %d) = %f\n", x, y, consumer);
			}
		}
#endif
	}
	if (0) {
		Func producer("producer_tile"), consumer("consumer_tile");
		producer(x,y) = sin(x*y);
		consumer(x,y) = (producer(x,y) + producer(x,y+1) + producer(x+1, y) + producer(x+1,y+1))/4;

		Var xo, yo, xi, yi;

		consumer.tile(x, y, xo, yo, xi, yi, 4, 4);

		producer.compute_at(consumer, xo);

		producer.trace_stores();
		consumer.trace_stores();

		consumer.realize({8, 8});
		consumer.print_loop_nest();
		consumer.print_loop_nest();
	}

}
