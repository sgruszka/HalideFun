#include "lesson_10_halide.h"
#include <HalideBuffer.h>
#include <stdio.h>

int main(int argc, char *argv[]) {
	Halide::Runtime::Buffer<uint8_t> input(640, 480), output(640, 480);
	int offset = 5;
	int error = brighter(input, offset, output);

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

	printf("Success\n");
	return 0;
}
