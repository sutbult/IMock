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

# TODO: Add timing of build/IMockTest.
# Builds the test executable and runs the automatic tests.
test: build-lcov-filter build
	find . -name "*.gcda" -type f -delete
	build/IMockTest ${filter}
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

# Builds a test executable using the single header.
build-with-single-header: merge-headers
	g++ \
		-std=c++11 \
		-g \
		-Itest/include \
		-IsingleHeader \
		test/src/IMock.cpp \
		test/src/IMockSecondary.cpp \
		test/src/main.cpp \
		-o build/IMockTestWithSingleHeader

# Builds a test executable using the single header and runs its automatic tests.
test-with-single-header: build-with-single-header
	bash -c "time build/IMockTestWithSingleHeader ${filter}"

# Runs both types of automatic tests.
test-both: test test-with-single-header

# Builds the program inside a Docker container.
docker-build:
	docker build -t imock .

# Builds the program and runs the automatic tests inside a Docker container.
docker-test: docker-build
	docker run -t imock make test

# Builds the program and runs the benchmarks inside a Docker container.
docker-benchmark: docker-build
	docker run -t imock make benchmark

# Builds the program and runs the automatic tests using the single header
# inside a Docker container.
docker-test-with-single-header: docker-build
	docker run -t imock make test-with-single-header

# Builds the program and runs both types of automatic tests inside a Docker
# container.
docker-test-both: docker-build
	docker run -t imock make test-both
