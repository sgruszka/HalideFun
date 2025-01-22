#include <Halide.h>
#include <cstdint>
#include <cstdlib>
#include <sys/types.h>

// using namespace Halide::Tools;
using namespace Halide;

class BrighterGenerator : public Generator<BrighterGenerator>
{
public:
	// GeneratorParam<bool> parallel{ "parallel", true };
	// GeneratorParam<float> scale{ "scale", 1.0f, 0.0f, 100.0f };

	enum class Layout {
		Planar,
		Interleaved,
		Either,
		Specialized,
	};
	GeneratorParam<Layout> layout{
		"layout",
		Layout::Planar,
		{ { "planar", Layout::Planar },
		  { "interleaved", Layout::Interleaved },
		  { "either", Layout::Either },
		  { "specialized", Layout::Specialized } }
	};

	Input<Buffer<uint8_t, 3>> input{ "input" };
	Input<uint8_t> offset{ "offset" };
	Output<Buffer<void, 3>> brighter{ "brighter" };

	Var x, y, c;

	void generate()
	{
		// brighter(x, y) = scale * (input(x, y) + offset);
		brighter(x, y, c) = input(x, y, c) + offset;
		brighter.vectorize(x, 16);

		switch (layout) {
		case Layout::Planar:
			break;
		case Layout::Interleaved:
			// RGBRGBRGBRGBRGBRGBRGBRGB
			input.dim(0).set_stride(3);
			input.dim(2).set_stride(1);
			brighter.dim(0).set_stride(3);
			brighter.dim(2).set_stride(1);

			input.dim(2).set_bounds(0, 3);
			brighter.dim(2).set_bounds(0, 3);
			brighter.reorder(c, x, y).unroll(c);
		case Layout::Either:
			// Unknown
			input.dim(0).set_stride(Expr());
			brighter.dim(0).set_stride(Expr());
			break;
		case Layout::Specialized:
			// Runtime recognition
			input.dim(0).set_stride(Expr());
			brighter.dim(0).set_stride(Expr());

			Expr input_is_planar =
				(input.dim(0).stride() == 1);
			Expr input_is_interleaved =
				(input.dim(0).stride() == 3 &&
				 input.dim(2).stride() == 1 &&
				 input.dim(2).extent() == 3);

			Expr output_is_planar =
				(brighter.dim(0).stride() == 1);
			Expr output_is_interleaved =
				(brighter.dim(0).stride() == 3 &&
				 brighter.dim(2).stride() == 1 &&
				 brighter.dim(2).extent() == 3);

			brighter.specialize(input_is_planar && output_is_planar);

			brighter.specialize(input_is_interleaved && output_is_interleaved)
				.reorder(c, x, y)
				.unroll(c);
			break;
		}

		// output(x, y) = cast(output.type(), brighter(x, y));
	}
};
HALIDE_REGISTER_GENERATOR(BrighterGenerator, brighter_generator);
