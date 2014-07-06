CC = clang
VERSION = 1.0.0

LIB = custache

SOURCES := $(realpath $(wildcard src/*.c))
OBJECTS := $(patsubst src/%.c, build/%.o, $(wildcard src/*.c))
EXAMPLES := main benchmark

SHARED_LIBS := apr-1

LINK_FLAGS := $(foreach lib,$(SHARED_LIBS),`pkg-config --cflags --libs $(lib)`) -lblock-utils

default: lib

lib: $(SOURCES)
	mkdir -p build
	cd build && \
	    $(CC) -fblocks -c -fPIC $(SOURCES) \
	    -I../include $(LINK_FLAGS)
	$(CC) -shared -Wl,-soname,lib$(LIB).so -o build/lib$(LIB).so $(OBJECTS)

examples: lib
	mkdir -p build
	$(foreach program,$(EXAMPLES),\
	$(CC) -std=c11 -fblocks examples/$(program).c -o build/$(program) -I./include -L./build -l$(LIB) \
	    $(LINK_FLAGS) -lBlocksRuntime &&) echo "finished building examples"

install:
	cp -R include/* /usr/local/include
	cp build/lib$(LIB)* /usr/local/lib

#3rdparty:
	## apr
	#cd 3rdparty/apr && ./configure && make && sudo make install && make clean
	## r3
	#cd 3rdparty/r3 && ./autogen.sh && ./configure && make && sudo make install && make clean
	## libmicrohttpd
	#cd 3rdparty/libmicrohttpd && ./configure && make && sudo make install && make clean
	## jansson
	#cd 3rdparty/jansson && ./configure && make && sudo make install && make clean
	## lpeg
	#cd 3rdparty/lpeg && make && sudo mv lpeg.so /usr/local/lib/ && make clean
	## luajit
	#cd 3rdparty/luajit && make && sudo make install && make clean
	
clean:
	rm -rf build

.PHONY: default main lib examples install clean 3rdparty

