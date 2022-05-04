# Trick used to get the absolute path to this makefile. Retrieved from:
# https://www.systutorials.com/how-to-get-the-full-path-and-directory-of-a-makefile-itself/
mkfile_path := $(abspath $(lastword $(MAKEFILE_LIST)))
mkfile_dir := $(abspath $(dir $(mkfile_path)))

# Set the default goal to build, which means running "make" will invoke build.
.DEFAULT_GOAL := build

# Initializes the repository by creating the build directory.
init:
	mkdir -p build

# Builds the test executable.
build: init
	cd build \
		&& cmake .. \
		&& $(MAKE)

# Builds lcovFilter.
build-lcov-filter: init
	g++ \
		test/script/lcovFilter.cpp \
		-o build/lcovFilter

# Builds the test executable and runs the automatic tests.
test: build-lcov-filter build
	find . -name "*.gcda" -type f -delete
	bash -c "time build/IMockTest ${filter}"
	find . -name "IMockSecondary.cpp.gcda" -type f -delete
	lcov \
		--capture \
		--directory . \
		--rc lcov_branch_coverage=1 \
		--output-file lcov.info.raw \
		--no-external \
		--exclude "${mkfile_dir}/test/*"
	build/lcovFilter lcov.info.raw lcov.info
	rm lcov.info.raw
	genhtml \
		lcov.info \
		--output-directory coverage \
		--branch-coverage

# Builds the test executable and runs the benchmarks.
benchmark: build
	find . -name "*.gcda" -type f -delete
	build/IMockTest [benchmark]

# Builds mergeHeaders.
build-merge-headers: init
	g++ \
		test/script/mergeHeaders.cpp \
		-o build/mergeHeaders

# Merges the headers into a single header.
merge-headers: build-merge-headers
	mkdir -p singleHeader
	build/mergeHeaders include IMock.hpp singleHeader/IMock.hpp

# Builds a test executable using the single header using a specified C++ version
# and a specified compiler.
build-with-single-header-base: merge-headers
	${compiler} \
		-std=${cppVersionStd} \
		-g \
		-Itest/include \
		-IsingleHeader \
		test/src/IMock.cpp \
		test/src/IMockSecondary.cpp \
		test/src/main.cpp \
		-lstdc++ \
		-lm \
		-o build/IMockTestWithSingleHeader${fileName}

# Builds a test executable using the single header using a specified C++ version
# and gcc.
build-with-single-header-version-gcc:
	$(MAKE) build-with-single-header-base \
		compiler=g++ \
		fileName="${cppVersionOut}Gcc"

# Builds a test executable using the single header using a specified C++ version
# and clang.
build-with-single-header-version-clang:
	$(MAKE) build-with-single-header-base \
		compiler=clang \
		fileName="${cppVersionOut}Clang"

# Builds a test executable using the single header using C++11 and gcc.
build-with-single-header-cpp11-gcc:
	$(MAKE) build-with-single-header-version-gcc \
		cppVersionStd=c++11 \
		cppVersionOut=Cpp11

# Builds a test executable using the single header using C++14 and gcc.
build-with-single-header-cpp14-gcc:
	$(MAKE) build-with-single-header-version-gcc \
		cppVersionStd=c++14 \
		cppVersionOut=Cpp14

# Builds a test executable using the single header using C++11 and clang.
build-with-single-header-cpp11-clang:
	$(MAKE) build-with-single-header-version-clang \
		cppVersionStd=c++11 \
		cppVersionOut=Cpp11

# Builds a test executable using the single header using C++14 and clang.
build-with-single-header-cpp14-clang:
	$(MAKE) build-with-single-header-version-clang \
		cppVersionStd=c++14 \
		cppVersionOut=Cpp14

# Builds a test executable using the single header and runs its automatic tests
# using C++11 and gcc.
test-with-single-header-cpp11-gcc: build-with-single-header-cpp11-gcc
	bash -c "time build/IMockTestWithSingleHeaderCpp11Gcc ${filter}"

# Builds a test executable using the single header and runs its automatic tests
# using C++14 and gcc.
test-with-single-header-cpp14-gcc: build-with-single-header-cpp14-gcc
	bash -c "time build/IMockTestWithSingleHeaderCpp14Gcc ${filter}"

# Builds a test executable using the single header and runs its automatic tests
# using C++11 and clang.
test-with-single-header-cpp11-clang: build-with-single-header-cpp11-clang
	bash -c "time build/IMockTestWithSingleHeaderCpp11Clang ${filter}"

# Builds a test executable using the single header and runs its automatic tests
# using C++14 and clang.
test-with-single-header-cpp14-clang: build-with-single-header-cpp14-clang
	bash -c "time build/IMockTestWithSingleHeaderCpp14Clang ${filter}"

# Runs all types of automatic tests.
test-all: test \
	test-with-single-header-cpp11-gcc \
	test-with-single-header-cpp14-gcc \
	test-with-single-header-cpp11-clang \
	test-with-single-header-cpp14-clang

# Builds the program inside a Docker container.
docker-build:
	docker build -t imock .

# Builds the program and runs the automatic tests inside a Docker container.
docker-test: docker-build
	docker run -t imock make test

# Builds the program and runs the benchmarks inside a Docker container.
docker-benchmark: docker-build
	docker run -t imock make benchmark

# Builds the program and runs all types of automatic tests inside a Docker
# container.
docker-test-all: docker-build
	docker run -t imock make test-all
