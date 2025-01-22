P=/home/stasiu/halide-install/
GEN_CMD=LD_LIBRARY_PATH=${P}/lib/ ./lesson16_generate -g brighter_generator -o .

# all: lesson16_generate lesson_16_run
all: lesson16_generate lesson_16_run

# %: %.cpp
# 	g++ $< -o $@ -g -I${P}/include/ -I${P}/share/tools/ -L/${P}/lib/ -lHalide `libpng-config --cflags --ldflags` -ljpeg -lpthread -ldl -std=c++17
#
lesson16_generate: lesson16_generate.cpp
	g++ $< ${P}/lib/libHalide_GenGen.a -o $@ -g -I${P}/include/ -I${P}/share/tools/ -L/${P}/lib/  -lHalide `libpng-config --cflags --ldflags` -ljpeg -lpthread -ldl -fno-rtti -std=c++17

brighten_planar.a: lesson16_generate
	${GEN_CMD} -f brighten_planar      target=host layout=planar
brighten_interleaved.a: lesson16_generate
	${GEN_CMD} -f brighten_interleaved target=host layout=interleaved
brighten_either.a: lesson16_generate
	${GEN_CMD} -f brighten_either      target=host layout=either
brighten_specialized.a: lesson16_generate
	${GEN_CMD} -f brighten_specialized target=host layout=specialized

lesson_16_run: lesson_16_run.cpp brighten_specialized.a brighten_either.a  brighten_planar.a brighten_interleaved.a
	g++ $^ -o $@ -g -I${P}/include/ -I${P}/share/tools/ -L/${P}/lib/ -lHalide `libpng-config --cflags --ldflags` -ljpeg -lpthread -ldl -std=c++17

.PHONY: clean

clean:
	rm -f lesson16_generate lesson_16_run  brighten_* halide_runtime_x86.o

