#include <Halide.h>
#include <fstream>
#include <stdio.h>

#include "halide_benchmark.h"
#include "halide_image_io.h"

using namespace Halide;

constexpr int IN_SIZE = 4263168;
constexpr int IN_STRIDE = 3904;
constexpr int IN_WIDTH = IN_STRIDE / 2;
constexpr int IN_HEIGHT = IN_SIZE / IN_STRIDE;

constexpr int OUT_SIZE = 8404032;
constexpr int OUT_STRIDE = 7696;
constexpr int OUT_WIDTH = OUT_STRIDE / 4;
constexpr int OUT_HEIGHT = OUT_SIZE / OUT_STRIDE;

inline double current_time()
{
	static auto start_time = Halide::Tools::benchmark_now().time_since_epoch();

	auto now = Halide::Tools::benchmark_now().time_since_epoch() - start_time;
	return std::chrono::duration_cast<std::chrono::microseconds>(now).count() / 1e3;
}

[[maybe_unused]]
static void diffrent_patterns()
{
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
}

#define LUT_SIZE 256

static uint8_t red_lut_values[LUT_SIZE] = {
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 14, 23, 29, 35, 38, 43, 46, 50, 53, 56, 59, 62, 65, 67, 70, 72, 74, 77, 79, 81, 83, 85, 87, 89, 91, 92, 94, 96, 98, 99, 101, 103, 105, 106, 108, 109, 111, 112, 114, 115, 117, 118, 120, 121, 122, 124, 125, 126, 128, 129, 130, 132, 133, 134, 135, 137, 138, 139, 140, 141, 143, 144, 145, 146, 147, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159, 161, 162, 162, 163, 165, 166, 167, 168, 169, 170, 171, 172, 172, 173, 174, 175, 176, 177, 178, 179, 180, 181, 182, 183, 184, 185, 186, 187, 188, 188, 189, 190, 191, 192, 193, 194, 195, 195, 196, 197, 198, 199, 200, 201, 201, 202, 203, 204, 205, 206, 206, 207, 208, 209, 210, 210, 211, 212, 213, 214, 214, 215, 216, 217, 218, 218, 219, 220, 221, 221, 222, 223, 224, 224, 225, 226, 227, 227, 228, 229, 230, 230, 231, 232, 233, 233, 234, 235, 235, 236, 237, 238, 238, 239, 240, 240, 241, 242, 242, 243, 244, 245, 245, 246, 247, 247, 248, 249, 249, 250, 251, 251, 252, 253, 253, 254, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255
};

static uint8_t blue_lut_values[LUT_SIZE] = {
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 20, 29, 37, 43, 48, 53, 58, 62, 65, 69, 72, 76, 79, 82, 85, 88, 90, 93, 96, 98, 101, 103, 106, 108, 110, 112, 115, 117, 119, 121, 123, 125, 127, 129, 131, 133, 134, 136, 138, 140, 142, 143, 145, 147, 148, 150, 152, 153, 155, 156, 158, 160, 161, 163, 164, 166, 167, 169, 170, 171, 173, 174, 176, 177, 179, 180, 181, 183, 184, 185, 187, 188, 189, 190, 192, 193, 194, 196, 197, 198, 199, 201, 202, 203, 204, 205, 207, 208, 209, 210, 211, 213, 214, 215, 216, 217, 218, 220, 221, 222, 223, 224, 225, 226, 227, 228, 229, 230, 232, 233, 234, 235, 236, 237, 238, 239, 240, 241, 242, 243, 244, 245, 246, 247, 248, 249, 250, 251, 252, 253, 254, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255
};

static uint8_t green_lut_values[LUT_SIZE] = {
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 16, 23, 28, 33, 37, 40, 43, 46, 49, 52, 55, 57, 59, 62, 64, 66, 68, 70, 72, 74, 76, 77, 79, 81, 83, 84, 86, 87, 89, 90, 92, 93, 95, 96, 98, 99, 101, 102, 103, 105, 106, 107, 108, 110, 111, 112, 113, 115, 116, 117, 118, 119, 120, 122, 123, 124, 125, 126, 127, 128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143, 144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 154, 155, 156, 157, 158, 159, 160, 161, 161, 162, 163, 164, 165, 166, 166, 167, 168, 169, 170, 170, 171, 172, 173, 174, 174, 175, 176, 177, 178, 178, 179, 180, 181, 181, 182, 183, 184, 184, 185, 186, 187, 187, 188, 189, 190, 190, 191, 192, 192, 193, 194, 195, 195, 196, 197, 197, 198, 199, 199, 200, 201, 202, 202, 203, 204, 204, 205, 206, 206, 207, 208, 208, 209, 210, 210, 211, 212, 212, 213, 213, 214, 215, 215, 216, 217, 217, 218, 219, 219, 220, 220, 221, 222, 222, 223, 224, 224, 225, 225, 226, 227, 227, 228, 228, 229, 230, 230, 231, 231, 232, 233, 233, 234, 234, 235, 236, 236, 237, 237, 238, 238, 239, 240, 240, 241, 241, 242, 242, 243, 244, 244, 245, 245, 246, 246, 247, 28, 248, 249, 249, 250, 250, 251, 251, 252, 252, 253, 254, 254
};

void apply_static_lut(Func &r, Func &g, Func &b, Func red16, Func green16, Func blue16)
{
	Var x, y;

	Buffer<uint8_t> red_lut_buf(red_lut_values, LUT_SIZE);
	Buffer<uint8_t> blue_lut_buf(blue_lut_values, LUT_SIZE);
	Buffer<uint8_t> green_lut_buf(green_lut_values, LUT_SIZE);

	Var i;
	Func red_lut, blue_lut, green_lut;

	red_lut(i) = red_lut_buf(i);
	blue_lut(i) = blue_lut_buf(i);
	green_lut(i) = green_lut_buf(i);

	Func red_raw("red_raw"), green_raw("green_raw"), blue_raw("blue_raw");

	blue_raw(x, y) = cast<uint8_t>(clamp(red16(x, y) / 4, 0, 255));
	red_raw(x, y) = cast<uint8_t>(clamp(blue16(x, y) / 4, 0, 255));
	green_raw(x, y) = cast<uint8_t>(clamp(green16(x, y) / 4, 0, 255));

	b(x, y) = blue_lut(blue_raw(x, y));
	r(x, y) = red_lut(red_raw(x, y));
	g(x, y) = green_lut(green_raw(x, y));
}

static void combine_and_save_image(Func r, Func g, Func b)
{
	Var x, y, c;

	/* Func debayered("debayered");
	debayered(x, y, c) = select(
		c == 0, r(x, y),
		c == 1, g(x, y),
		c == 2, b(x, y),
		255); */

	// Scheduling (Optimize for parallel execution)
	// debayered.parallel(y).vectorize(x, 16);
	/*
	printf("OUT %d x %d OUT_SIZE %d OUT_STRIDE %d \n", OUT_WIDTH, OUT_HEIGHT, OUT_SIZE, OUT_STRIDE);
	// Halide::Buffer<uint8_t> out = debayered.realize({ OUT_WIDTH, OUT_HEIGHT, 4 });
	Halide::Buffer<uint8_t> out = debayered.realize({ IN_WIDTH, IN_HEIGHT, 4 });
	// Halide::Buffer<uint8_t> out = debayered.realize({ STRIDE, SIZE / STRIDE });
	Tools::save_image(out, "OUT_FRAME.png"); */
}

int halide_debayer()
{
	std::ifstream inputFile("images/INPUT_FRAME", std::ios::in | std::ios::binary);

	uint16_t *buf = new uint16_t[IN_SIZE / 2];
	inputFile.read((char *)buf, IN_SIZE);

	if (inputFile.gcount() != IN_SIZE) {
		fprintf(stderr, " Size does not match:  %ld != %d\n", inputFile.gcount(), IN_SIZE);
		return -1;
	}

	printf("IN %d x %d IN_SIZE %d IN_STRIDE %d \n", IN_WIDTH, IN_HEIGHT, IN_SIZE, IN_STRIDE);
	Buffer<uint16_t> input(buf, IN_WIDTH, IN_HEIGHT);

	Var x("x"), y("y"), c("c");
	// GRGRGR..
	// BGBGBG..
	Expr is_green1 = ((x & 1) == 0) && ((y & 1) == 0);
	Expr is_red = ((x & 1) == 1) && ((y & 1) == 0);
	Expr is_blue = ((x & 1) == 0) && ((y & 1) == 1);
	Expr is_green2 = ((x & 1) == 1) && ((y & 1) == 1);

	// Interpolation for each color channel
	Func red16("red16"), green16("green16"), blue16("blue16");
	Func raw("raw"), padded16("padded16");

	// constexpr int INPUT_W = IN_STRIDE ; // HOW THIS WORKED ?
	// constexpr int INPUT_H = IN_SIZE / IN_STRIDE;
	// Expr padx = clamp(x, 0, INPUT_W - 1);
	// Expr pady = clamp(y, 0, INPUT_H - 1);
	Expr padx = clamp(x, 0, IN_WIDTH - 1);
	Expr pady = clamp(y, 0, IN_HEIGHT - 1);

	// Red channel
	// padded16(x, y) = cast<uint16_t>(input(padx, pady));
	raw(x, y) = input(padx, pady);
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

	Func red("red"), green("green"), blue("blue");

	RDom r(0, IN_WIDTH, 0, IN_HEIGHT);
	Func sum_r, sum_g, sum_b;
	sum_r() = cast<uint64_t>(0);
	sum_g() = cast<uint64_t>(0);
	sum_b() = cast<uint64_t>(0);

	Expr is_r = ((r.x & 1) == 0) && ((r.y & 1) == 0);
	Expr is_g = (((r.x + r.y) & 1) == 1);
	Expr is_b = ((r.x & 1) == 1) && ((r.y & 1) == 1);

	sum_r() += select(is_r, cast<uint64_t>(raw(r.x, r.y)), 0);
	sum_g() += select(is_g, cast<uint64_t>(raw(r.x, r.y)), 0);
	sum_b() += select(is_b, cast<uint64_t>(raw(r.x, r.y)), 0);

	// sum_r.compute_root();
	// sum_g.compute_root();
	// sum_b.compute_root();
	double t1 = current_time();

	Halide::Buffer<uint64_t> sum_r_buf = sum_r.realize();
	Halide::Buffer<uint64_t> sum_g_buf = sum_g.realize();
	Halide::Buffer<uint64_t> sum_b_buf = sum_b.realize();
	printf("SUMS: R %llu G %llu B %llu\n", (unsigned long long)sum_r_buf(0), (unsigned long long)sum_g_buf(0), (unsigned long long)sum_b_buf(0));

	float sum_r_val = sum_r_buf(0);
	float sum_g_val = sum_g_buf(0) / 2.0;
	float sum_b_val = sum_b_buf(0);

	float gain_r = sum_r_val / sum_g_val;
	float gain_b = sum_b_val / sum_g_val;

	printf("GAINS: R %lf G %lf B %lf\n", sum_r_val / sum_g_val, 1.0, sum_b_val / sum_g_val);

	constexpr bool dynamic_lut = true;
	constexpr float black_level = 16;
	constexpr float divisor = 256 - black_level - 1.0f;

	if (dynamic_lut) {
		Func lut, gamma, gamma_clamped;
		Var i;

		gamma(i) = pow((i - black_level) / divisor, 0.5f) * 255.0f;
		gamma_clamped(i) = cast<uint8_t>(clamp(gamma(i), 0, 255));

		lut(i) = select(i < black_level, 0, gamma_clamped(i));
		Func lut_b, lut_r, lut_g;

		lut_b(i) = lut(cast<uint8_t>(min(i * gain_b, 255)));
		lut_g(i) = lut(i);
		lut_r(i) = lut(cast<uint8_t>(min(i * gain_r, 255)));

		blue(x, y) = cast<uint8_t>(lut_b(blue16(x, y) / 4));
		red(x, y) = cast<uint8_t>(lut_r(red16(x, y) / 4));
		green(x, y) = cast<uint8_t>(lut_g(green16(x, y) / 4));
	} else {
		apply_static_lut(red, green, blue, red16, green16, blue16);
	}

	double t2 = current_time();
	Func debayered("debayered");
	debayered(x, y, c) = select(
		c == 0, red(x, y),
		c == 1, green(x, y),
		c == 2, blue(x, y),
		255);

	// Scheduling (Optimize for parallel execution)

	debayered.compute_root();

	debayered.reorder(c, x, y).bound(c, 0, 4).unroll(c);
	// debayered.parallel(y).vectorize(x, 16);
	// debayered.parallel(y);

	// Combine into final RGB image
	printf("OUT %d x %d OUT_SIZE %d OUT_STRIDE %d \n", OUT_WIDTH, OUT_HEIGHT, OUT_SIZE, OUT_STRIDE);

	// Halide::Buffer<uint8_t> out = debayered.realize({ OUT_WIDTH, OUT_HEIGHT, 4 });
	Halide::Buffer<uint8_t> out = debayered.realize({ IN_WIDTH, IN_HEIGHT, 4 });

	double t3 = current_time();

	// double sum_r_val = sum_r_buf(0);
	// double sum_g_val = sum_g_buf(0) / 2.0;
	// double sum_b_val = sum_b_buf(0);

	printf("Sums done in %1.4f ms debayering done in %1.4f ms\n", (t2 - t1) / 100, (t3 - t2) / 100);

	Tools::save_image(out, "OUT_FRAME.png");
	// combine_and_save_image(red, green, blue);
	return 0;
}

int read_output()
{
	std::ifstream inputFile("images/OUTPUT_FRAME", std::ios::in | std::ios::binary);

	// constexpr int SIZE = 8404032;
	// constexpr int STRIDE = 7696;
	uint8_t *buf = new uint8_t[OUT_SIZE];
	inputFile.read((char *)buf, OUT_SIZE);

	if (inputFile.gcount() != OUT_SIZE) {
		fprintf(stderr, "Size does not match\n");
		return -1;
	}

	// 	for (int i = 0; i < STRIDE; i += 4) {
	// 		uint8_t *pix = buf + i;
	// 		printf("%d %d %d", pix[0], pix[1], pix[2]);
	// 	}
	constexpr int W = OUT_WIDTH; //STRIDE / 4;
	constexpr int H = OUT_HEIGHT; //SIZE / STRIDE;
	if (0) {
		Buffer<uint8_t> out1_buf({ W, H, 3 });
		for (int y = 0; y < H; y++) {
			for (int x = 0; x < W; x++) {
				uint8_t *pix = buf + y * OUT_STRIDE + x * 4;
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
