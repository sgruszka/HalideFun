P=/home/stasiu/halide-install/

all: lesson15_generate lesson_15_run

# %: %.cpp
# 	g++ $< -o $@ -g -I${P}/include/ -I${P}/share/tools/ -L/${P}/lib/ -lHalide `libpng-config --cflags --ldflags` -ljpeg -lpthread -ldl -std=c++17
#

lesson15_generate: lesson15_generate.cpp
	g++ $< ${P}/lib/libHalide_GenGen.a -o $@ -g -I${P}/include/ -I${P}/share/tools/ -L/${P}/lib/  -lHalide `libpng-config --cflags --ldflags` -ljpeg -lpthread -ldl -fno-rtti -std=c++17

my_generator_multi.a: lesson15_generate
	./lesson_15_generators_usage.sh

lesson_15_run: lesson_15_run.cpp my_generator_multi.a
	g++ $^ -o $@ -g -I${P}/include/ -I${P}/share/tools/ -L/${P}/lib/ -lHalide `libpng-config --cflags --ldflags` -ljpeg -lpthread -ldl -std=c++17

.PHONY: clean

clean:
	rm -f lesson15_generate lesson_15_run  my_generator* second_generator* halide_runtime_x86.o

