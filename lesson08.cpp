#include <Halide.h>
#include <stdio.h>
// #include "halide_image_io.h"

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
	}
	if (1) {
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
