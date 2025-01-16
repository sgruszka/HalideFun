#include <Halide.h>
#include <cassert>
#include <sys/types.h>

using namespace Halide;

Expr average(Expr a, Expr b);

int main(int argc, char *argv[])
{
	Type valid_halide_types[] = {
		UInt(8), UInt(16), UInt(32), UInt(64),
		Int(8), Int(16), Int(32), Int(64),
		Float(32), Float(64), Handle()
	};

	{
		assert(UInt(8).bits() == 8);
		assert(Int(8).is_int());
	}
	{
		Type t = UInt(8);
		t = t.with_bits(t.bits() * 2);
		assert(t == UInt(16));

		assert(type_of<float>() == Float(32));

		Var x;
		Expr e = x;
		assert(e.type() == Int(32));

		assert(sin(x).type() == Float(32));

		assert(cast(UInt(8), x).type() == UInt(8));
		assert(cast<uint8_t>(x).type() == UInt(8));

		Func f;
		f(x) = cast<uint8_t>(x);
		assert(f.types()[0] == UInt(8));

		Func mf;
		mf(x) = { x, sin(x) };
		assert(mf.types()[0] == Int(32));
		assert(mf.types()[1] == Float(32));
	}
	{
		// Type promotion rules.
		Var x;
		Expr u8 = cast<uint8_t>(x);
		Expr u16 = cast<uint16_t>(x);
		Expr u32 = cast<uint32_t>(x);
		Expr u64 = cast<uint64_t>(x);
		Expr s8 = cast<int8_t>(x);
		Expr s16 = cast<int16_t>(x);
		Expr s32 = cast<int32_t>(x);
		Expr s64 = cast<int64_t>(x);
		Expr f32 = cast<float>(x);
		Expr f64 = cast<double>(x);

		for (Type t : valid_halide_types) {
			if (t == Handle())
				continue;
			Expr e = cast(t, x);
			assert((e + e).type() == t);
			assert((e + e).type() == e.type());
		}

		assert((u8 + f32).type() == Float(32));
		assert((f32 + s64).type() == Float(32));
		assert((u16 + f64).type() == Float(64));
		assert((f64 + s32).type() == Float(64));

		assert((f64 + f32).type() == Float(64));

		// Expr bad = u8 + 257;

		// int32_t result32 = evaluate<int>(cast<int32_t>(cast<uint8_t>(255)));
		// assert(result32 == 255);

		// uint16_t result16 = evaluate<uint16_t>(cast<uint16_t>(cast<int8_t>(-1)));
		// assert(result16 == 65535);
	}
	// The Handle type
	{
		Var x;
		assert(type_of<void *>() == Handle());
		assert(type_of<const char *const **>() == Handle());
		assert(Handle().bits() == 64);
	}

	{
		Var x;
		assert(average(cast<float>(x), 3.0f).type() == Float(32));
		assert(average(x, 3).type() == Int(32));
		assert(average(cast<uint8_t>(x), cast<uint8_t>(3)).type() == UInt(8));
	}

	printf("Success!\n");
	return 0;
}

Expr average(Expr a, Expr b)
{
	assert(a.type() == b.type());

	if (a.type().is_float() || a.type().bits() == 64) {
		return (a + b) / 2;
	}
	// Avoid overflow
	Type narrow = a.type();
	Type wider = narrow.with_bits(narrow.bits() * 2);
	Expr aw = cast(wider, a);
	Expr bw = cast(wider, b);
	return cast(narrow, (a + b) / 2);
}
