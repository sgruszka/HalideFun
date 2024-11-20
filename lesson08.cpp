#include <cstdint>
#include <stdio.h>
#include <cmath>
#include <Halide.h>
#include "halide_image_io.h"
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
	if (0) {
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

				producer_storage[(y+1) & 1][x+1] = sinf((x+1)*(y+1));
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

#ifdef C_EVALUATION
		float producer_storage[5][5];

		for (int yo = 0; yo < 2; yo++) {
			for (int xo = 0; xo < 2; xo++) {
				int yb = yo*4;
				int xb = xo*4;

				for (int py = yb; py < yb+5; py++) {
					for (int px = xb; px < xb+5; px++) {
						producer_storage[py - yb][px - xb] = sinf(px*py);
						printf("Store producer (%d, %d) = %f\n", px - xb, py - yb, sinf(px*py));
					}
				}

				for (int yi = 0; yi < 4; yi++) {
					for (int xi = 0; xi < 4; xi++) {
						int x = xb + xi;
						int y = yb + yi;
						float consumer = (
							producer_storage[yi][xi] +
							producer_storage[yi][xi+1] +
							producer_storage[yi+1][xi] +
							producer_storage[yi+1][xi+1]
								)/4;
						printf("Evaluate consumer (%d, %d) = %f\n", x, y, consumer);
					}
				}
			}
		}
#endif
	}
	if (1) {
		Func producer("producer"), consumer("consumer");
		producer(x,y) = sin(x*y);
		consumer(x,y) = (producer(x,y) + producer(x,y+1) + producer(x+1, y) + producer(x+1,y+1))/4;

		Var yi, yo;

		consumer.split(y, yo, yi, 16);
		consumer.parallel(yo);
		consumer.vectorize(x, 4);

		producer.store_at(consumer, yo);
		producer.compute_at(consumer, yi);
		producer.vectorize(x, 4);

		Buffer<float> res = consumer.realize({160, 160});

		Func out;
		float factor = 255.0/2.0;
		out(x,y) = cast<uint8_t>(factor*(res(x,y) + 1.0f));

		Buffer<uint8_t> out_buf = out.realize({res.width(), res.height()});
		Tools::save_image(out_buf, "out.png");
	}
}
