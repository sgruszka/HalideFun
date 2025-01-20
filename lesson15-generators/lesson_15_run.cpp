#include <HalideBuffer.h>
#include <stdio.h>

#include "halide_image_io.h"
#include "my_generator_avx.h"

int main(int argc, char *argv[])
{
	Halide::Runtime::Buffer<uint8_t> input = Halide::Tools::load_image("../images/gray.png");
	Halide::Runtime::Buffer<uint8_t> output(input.width(), input.height());
	uint8_t offset = 5;
	int error = my_generator_avx(offset, input, output);

	if (error) {
		fprintf(stderr, "Halide error %d\n", error);
		return -1;
	}

	for (int y = 0; y < 480; y++) {
		for (int x = 0; x < 640; x++) {
			uint8_t in_val = input(x, y);
			uint8_t out_val = output(x, y);
			uint8_t val = in_val + offset;
			if (val != out_val) {
				printf("Output was %d instead %d for (%d, %d)\n",
				       out_val, val, x, y);
				return -1;
			}
		}
	}

	Halide::Tools::save_image(output, "out_from_generator.png");
	printf("Success\n");
	return 0;
}
