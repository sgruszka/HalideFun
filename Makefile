P=/home/stasiu/halide-install/

SRCS := $(wildcard *.cpp)
TGTS := $(SRCS:.cpp=)

all: $(TGTS)

%: %.cpp
	g++ $< -o $@ -g -I${P}/include/ -I${P}/share/tools/ -L/${P}/lib/ -lHalide `libpng-config --cflags --ldflags` -ljpeg -lpthread -ldl -std=c++17

lesson_10_halide.a: lesson10_generate
	LD_LIBRARY_PATH=${P}/lib ./lesson10_generate

lesson_10_run: lesson_10_run.cpp lesson_10_halide.a
	g++ $< lesson_10_halide.a -o $@ -g -I${P}/include/ -I${P}/share/tools/ -L/${P}/lib/ -std=c++17


.PHONY: clean

clean:
	rm -f $(TGTS)
	rm -f lesson_10_halide.a

