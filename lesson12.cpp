#include <stdio.h>
#include <Halide.h>

// #include "clock.h"
#include "halide_image_io.h"

using namespace Halide;
using namespace Halide::Tools;

Target find_gpu_target() {
	// Start with a target suitable for the machine you're running this on.
	Target target = get_host_target();

	std::vector<Target::Feature> features_to_try;
	features_to_try.push_back(Target::OpenCL);
	features_to_try.push_back(Target::Vulkan);

	for (Target::Feature f : features_to_try) {
		Target new_target = target.with_feature(f);
		if (host_supports_target_device(new_target)) {
			printf("Found target %d %d %d\n", new_target.os, new_target.arch, new_target.has_gpu_feature());
			return new_target;
		}
	}

	fprintf(stderr, "Requested GPU(s) features are not supported.\n");
	return target;
}

Var x,y,c,i, ii, xo, yo, xi, yi;

class Brighter {
public:
	Func brighter;
	Buffer<uint8_t> input;

	Brighter(Buffer<uint8_t> in, uint8_t off) : input(in) {
		brighter(x,y) = clamp(input(x,y) + off, 0, 255);
	}

	void schedule_for_gpu() {
		Target target = find_gpu_target();
		assert(target.has_gpu_feature());
		// Target target = get_host_target();

		target.set_feature(Target::Debug);
		target.set_feature(Target::VulkanInt8);

		Var yi, yo;
		brighter.split(y, yo, yi, 16).parallel(yo);
		brighter.reorder(yi, x, yo);
		// Var x,y, xo,yo, xi,yi;
		// brighter.reorder(c,x,y).bound(c, 0, 3).unroll(c);
		brighter.gpu_tile(x, y, xo, yo, xi, yi, 16, 16);
		brighter.compile_jit(target);
	}
};

class MyPipeline {
public:
	Func lut, padded, padded16, sharpen, curved;
	Buffer<uint8_t> input;

	MyPipeline(Buffer<uint8_t> in) : input(in) {
		// Define LUT (Look Up Table) - it will be a gamma curve.
		lut(i) = cast<uint8_t>(clamp(pow(i / 255.0f, 1.2f) * 255.0f, 0, 255));
		// lut(i) = cast<uint8_t>(i);

		Expr padx = clamp(x, 0, input.width() - 1);
		Expr pady = clamp(y, 0, input.height() - 1);
		padded(x,y,c) = input(padx, pady, c);

		padded16(x,y,c) = cast<int16_t>(padded(x,y,c));

		// Sharp with five-tap filter
		sharpen(x, y, c) = (padded16(x,y,c)*2) -
					(padded16(x-1, y, c) +
					 padded16(x, y-1, c) +
					 padded16(x+1, y, c) +
					 padded16(x, y+1, c)/4);

		// sharpen(x,y,c) = padded16(x,y,c);
		// Apply the LUT
		curved(x, y, c) = lut(sharpen(x,y,c));
	}

	void schedule_for_cpu() {
		lut.compute_root();

		// color channels innermost, mark there will be 3 of them and unroll
		// curved.reorder(c, x, y).bound(c, 0, 3).unroll(c);

		Var yi, yo;
		curved.split(y, yo, yi, 16).parallel(yo);

		// compute sharpen as needed per scanline of curved.
		sharpen.compute_at(curved, yi);
		sharpen.vectorize(x, 8);

		padded.store_at(curved, yo).compute_at(curved, yi);

		padded.vectorize(x, 16);
		Target target = get_host_target();
		curved.compile_jit(target);
	}

	void scheudle_for_gpu() {
		Target target = find_gpu_target();
		assert(target.has_gpu_feature());
		target.set_feature(Target::Debug);
		target.set_feature(Target::VulkanInt8);
		target.set_feature(Target::VulkanInt16);
		printf("Target %s\n", target.to_string().c_str());

		lut.compute_root();

		Var block, thread;
		lut.split(i, block, thread, 16);
		lut.gpu_blocks(block).gpu_threads(thread);
		// lut.gpu_tile(i, block, thread, 16);

		//curved.reorder(c,x,y).bound(c, 0, 3).unroll(c);

		// Compute curved in 2D 8x8 tiles using the GPU.
		curved.gpu_tile(x, y, xo, yo, xi, yi, 8, 8);

		padded.compute_at(curved, xo);
		padded.gpu_threads(x, y);

		curved.compile_jit(target);
	}
};

int main() {
	Buffer<uint8_t> input = load_image("images/rgb.png");
	Buffer<uint8_t> input_gray = load_image("images/gray.png");

	// Allocated an image that will store the correct output
	Buffer<uint8_t> reference_output(input.width(), input.height(), input.channels());
	Buffer<uint8_t> gpu_output(input.width(), input.height(), input.channels());
	Buffer<uint8_t> gpu_output_gray(input_gray.width(), input_gray.height());

	printf("Running pipeline on CPU\n");
	MyPipeline p1(input);
	p1.schedule_for_cpu();
	p1.curved.realize(reference_output);
	save_image(reference_output, "out_cpu.png");

	// MyPipeline p2(input);
	// p2.scheudle_for_gpu();
	// p2.curved.realize(gpu_output);
	// save_image(gpu_output, "out_gpu.png");

	Brighter b(input_gray, 10);
	b.schedule_for_gpu();
	b.brighter.realize(gpu_output_gray);
	save_image(gpu_output_gray, "out_gpu.png");

	return 0;
}