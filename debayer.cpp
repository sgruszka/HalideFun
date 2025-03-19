#include <Halide.h>
#include <fstream>
#include <stdio.h>

#include "halide_image_io.h"

using namespace Halide;

int halide_debayer()
{
	constexpr int SIZE = 4263168;
	constexpr int STRIDE = 3904;
	std::ifstream inputFile("images/INPUT_FRAME", std::ios::in | std::ios::binary);

	uint16_t *buf = new uint16_t[SIZE / 2];
	inputFile.read((char *)buf, SIZE);

	if (inputFile.gcount() != SIZE) {
		fprintf(stderr, " Size does not match:  %ld != %d\n", inputFile.gcount(), SIZE);
		return -1;
	}

	Var x("x"), y("y"), c("c");

	// ImageParam raw(UInt(16), 2); // Assume 16-bit raw Bayer input
	Buffer<uint16_t> input(buf, STRIDE / 2, SIZE / STRIDE);

	// Define positions for RGGB pattern
	// Expr is_red = ((x % 2) == 0) && ((y % 2) == 0);
	// Expr is_green1 = ((x % 2) == 0) && ((y % 2) == 0);
	// Expr is_green2 = ((x % 2) == 1) && ((y % 2) == 1);
	// Expr is_blue = ((x % 2) == 0) && ((y % 2) == 1);

	// GBGBGB..
	// RGRGRG..
	// Expr is_green1 = ((x % 2) == 0) && ((y % 2) == 0);
	// Expr is_blue = ((x % 2) == 1) && ((y % 2) == 0);
	// Expr is_red = ((x % 2) == 0) && ((y % 2) == 1);
	// Expr is_green2 = ((x % 2) == 1) && ((y % 2) == 1);

	// red16(x, y) = select(
	// 	is_red, raw(x, y),
	// 	is_blue, (raw(x - 1, y - 1) + raw(x + 1, y - 1) + raw(x - 1, y + 1) + raw(x + 1, y + 1)) / 4,
	// 	is_green1, (raw(x, y - 1) + raw(x, y + 1)) / 2,
	// 	/* is green2 */ (raw(x - 1, y) + raw(x + 1, y)) / 2);
	//
	// // Green channel
	// green16(x, y) = select(
	// 	is_green1 || is_green2, raw(x, y),
	// 	/* is_red || is_blue */ (raw(x, y - 1) + raw(x, y + 1) + raw(x - 1, y) + raw(x + 1, y)) / 4);
	//
	// // Blue channel
	// blue16(x, y) = select(
	// 	is_blue, raw(x, y),
	// 	is_green1, (raw(x - 1, y) + raw(x - 1, y)) / 2,
	// 	is_green2, (raw(x, y - 1) + raw(x, y + 1)) / 2,
	// 	/*is_red,*/ (raw(x - 1, y - 1) + raw(x + 1, y - 1) + raw(x - 1, y + 1) + raw(x + 1, y + 1)) / 4);

	// GRGRGR..
	// BGBGBG..
	Expr is_green1 = ((x % 2) == 0) && ((y % 2) == 0);
	Expr is_red = ((x % 2) == 1) && ((y % 2) == 0);
	Expr is_blue = ((x % 2) == 0) && ((y % 2) == 1);
	Expr is_green2 = ((x % 2) == 1) && ((y % 2) == 1);
	// Interpolation for each color channel
	Func red16("red"), green16("green"), blue16("blue");
	Func red("red"), green("green"), blue("blue");
	Func raw("raw"), padded16("padded16");

	constexpr int INPUT_W = STRIDE;
	constexpr int INPUT_H = SIZE / STRIDE;
	Expr padx = clamp(x, 0, INPUT_W - 1);
	Expr pady = clamp(y, 0, INPUT_H - 1);

	// Red channel
	padded16(x, y) = cast<uint16_t>(input(padx, pady));
	raw(x, y) = padded16(x, y);
	red16(x, y) = select(
		is_red, raw(x, y),
		is_blue, (raw(x - 1, y - 1) + raw(x + 1, y - 1) + raw(x - 1, y + 1) + raw(x + 1, y + 1)) / 4,
		is_green1, (raw(x - 1, y) + raw(x + 1, y)) / 2,
		/* is_green2 */ (raw(x, y - 1) + raw(x, y + 1)) / 2);

	// Green channel
	green16(x, y) = select(
		is_green1 || is_green2, raw(x, y),
		/* is_red || is_blue */ (raw(x, y - 1) + raw(x, y + 1) + raw(x - 1, y) + raw(x + 1, y)) / 4);

	// Blue channel
	blue16(x, y) = select(
		is_blue, raw(x, y),
		is_green1, (raw(x, y - 1) + raw(x, y + 1)) / 2,
		is_green2, (raw(x - 1, y) + raw(x - 1, y)) / 2,
		/*is_red,*/ (raw(x - 1, y - 1) + raw(x + 1, y - 1) + raw(x - 1, y + 1) + raw(x + 1, y + 1)) / 4);

	Func lut;
	Var i;
	lut(i) = cast<uint8_t>(clamp(pow(i / 256.0f, 0.5f) * 255.0f, 0, 255));

	blue(x, y) = cast<uint8_t>(lut(blue16(x, y) / 4));
	red(x, y) = cast<uint8_t>(lut(red16(x, y) / 4));
	green(x, y) = cast<uint8_t>(lut(green16(x, y) / 4));

	// Combine into final RGB image
	Func debayered("debayered");
	debayered(x, y, c) = select(
		c == 0, red(x, y),
		c == 1, green(x, y),
		c == 2, blue(x, y),
		255);

	// Scheduling (Optimize for parallel execution)
	// debayered.parallel(y).vectorize(x, 16);

	constexpr int OUT_SIZE = 8404032;
	constexpr int OUT_STRIDE = 7696;

	//debayered(x, y) = red(x, y);

	constexpr int W = OUT_STRIDE / 4;
	constexpr int H = OUT_SIZE / OUT_STRIDE;
	printf("%d x %d SIZE %d STRIDE %d \n", W, H, OUT_SIZE, OUT_STRIDE);
	Halide::Buffer<uint8_t> out = debayered.realize({ W, H, 4 });
	// Halide::Buffer<uint8_t> out = debayered.realize({ STRIDE, SIZE / STRIDE });
	Tools::save_image(out, "OUT_FRAME.png");
	return 0;
}

int read_output()
{
	std::ifstream inputFile("images/OUTPUT_FRAME", std::ios::in | std::ios::binary);

	constexpr int SIZE = 8404032;
	constexpr int STRIDE = 7696;
	uint8_t *buf = new uint8_t[SIZE];
	inputFile.read((char *)buf, SIZE);

	if (inputFile.gcount() != SIZE) {
		fprintf(stderr, "Size does not match\n");
		return -1;
	}

	// 	for (int i = 0; i < STRIDE; i += 4) {
	// 		uint8_t *pix = buf + i;
	// 		printf("%d %d %d", pix[0], pix[1], pix[2]);
	// 	}
	constexpr int W = STRIDE / 4;
	constexpr int H = SIZE / STRIDE;
	if (0) {
		Buffer<uint8_t> out1_buf({ W, H, 3 });
		for (int y = 0; y < H; y++) {
			for (int x = 0; x < W; x++) {
				uint8_t *pix = buf + y * STRIDE + x * 4;
				out1_buf(x, y, 0) = pix[0];
				out1_buf(x, y, 1) = pix[1];
				out1_buf(x, y, 2) = pix[2];
			}
		}
		printf("\nSaving first image!\n");
		Halide::Tools::save_image(out1_buf, "frame_converted.png");
	} else {
		constexpr int width = W;
		constexpr int height = H;
		constexpr int channels = 4;

		Buffer<uint8_t> buffer = Buffer<uint8_t>::make_interleaved(buf, width, height, channels);
		Halide::Tools::save_image(buffer, "GOOD_FRAME.png");
	}

	return 0;
}

int main(int argc, char *argv[])
{
	halide_debayer();
	read_output();
	return 0;
}
