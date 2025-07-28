# Simple Makefile for zxlib tests
# Wrapper around CMake build system

.PHONY: all build test clean test-verbose test-filter

# Default target
all: build

# Build the library and tests
build:
	mkdir -p build
	cd build && cmake -DCMAKE_BUILD_TYPE=Debug ..
	cd build && make

# Run C++ tests
test: build
	cd build && GTEST_COLOR=1 ASAN_OPTIONS=detect_leaks=0 ctest -VV

# Clean build artifacts
clean:
	rm -rf build

# Run tests with verbose output on failure
test-verbose: build
	cd build && GTEST_COLOR=1 ASAN_OPTIONS=detect_leaks=0 ctest -VV --output-on-failure

# Run specific test
test-filter: build
	cd build && ./zxlib_tests --gtest_filter="$(FILTER)"