P=/home/stasiu/halide-install/

SRCS := $(wildcard *.cpp)
TGTS := $(SRCS:.cpp=)

all: $(TGTS)

%: %.cpp
	g++ $< -o $@ -g -I${P}/include/ -I${P}/share/tools/ -L/${P}/lib/ -lHalide `libpng-config --cflags --ldflags` -ljpeg -lpthread -ldl -std=c++17

.PHONY: clean

clean:
	rm -f $(TGTS)

