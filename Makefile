CC = clang

default:
	mkdir -p build
	cd build && CC="$(CC)" cmake .. && make

install:
	cd build && make install

clean:
	rm -rf build

.PHONY: default clean
