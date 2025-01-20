#include <Halide.h>
#include <cstdint>
#include <cstdlib>
#include <sys/types.h>

// using namespace Halide::Tools;
using namespace Halide;

class MyGenerator : public Generator<MyGenerator>
{
public:
	Input<uint8_t> offset{ "offset" };
	Input<Buffer<uint8_t, 2>> input{ "input" };

	Output<Buffer<uint8_t, 2>> brighter{ "brighter" };

	Var x, y;

	void generate()
	{
		brighter(x, y) = input(x, y) + offset;
		brighter.vectorize(x, 16).parallel(y);
	}
};
HALIDE_REGISTER_GENERATOR(MyGenerator, my_generator);

class SecondGenerator : public Generator<SecondGenerator>
{
public:
	GeneratorParam<bool> parallel{ "parallel", true };
	GeneratorParam<float> scale{ "scale", 1.0f, 0.0f, 100.0f };

	enum class Rotation { None,
			      Clockwise,
			      CounterClockwise };
	GeneratorParam<Rotation> rotation{
		"rotation",
		Rotation::None,
		{ { "none", Rotation::None },
		  { "cw", Rotation::Clockwise },
		  { "ccw", Rotation::CounterClockwise } }
	};

	Input<uint8_t> offset{ "offset" };
	Input<Buffer<uint8_t, 2>> input{ "input" };
	Output<Buffer<void, 2>> output{ "output" };

	Var x, y;

	void generate()
	{
		Func brighter;

		brighter(x, y) = scale * (input(x, y) + offset);

		Func rotated;

		switch (rotation) {
		case Rotation::None:
			rotated(x, y) = brighter(x, y);
			break;
		case Rotation::Clockwise:
			rotated(x, y) = brighter(y, 100 - x);
			break;
		case Rotation::CounterClockwise:
			rotated(x, y) = brighter(100 - y, x);
			break;
		}

		output(x, y) = cast(output.type(), rotated(x, y));
	}
};
HALIDE_REGISTER_GENERATOR(SecondGenerator, second_generator);
