# custache

[Mustache](http://mustache.github.io/) templates for C.

## Dependencies

1. [Apache Portable Runtime](https://apr.apache.org/)
2. [clang](http://clang.llvm.org/) with [blocks extension](http://en.wikipedia.org/wiki/Blocks_%28C_language_extension%29)

On Ubuntu, the dependencies can all be installed via `apt-get`:

    sudo apt-get install libapr1-dev clang libblocksruntime-dev

## Build / Install

To build:

    make

The examples will be put under `build/bin/`.

To install:

    sudo make install

## Benchmark

While the development is still in early stage, preliminary benchmarks indicate that `custache` is around twice as fast as its JavaScript equivalent and 20 times faster than the Python equivalent.

## Status

WIP.  See [examples](/examples) for a sneak peak.

