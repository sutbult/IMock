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
build-script: init
	g++ \
		test/script/lcovFilter.cpp \
		-o build/lcovFilter

# Builds the test executable and runs the automatic tests.
test: build-script build
	find . -name "*.gcda" -type f -delete
	build/IMockTest ${filter}
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

# Builds the program inside a Docker container.
docker-build:
	docker build -t imock .

# Builds the program and runs the automatic tests inside a Docker container.
docker-test: docker-build
	docker run -t imock make test

# Builds the program and runs the benchmarks inside a Docker container.
docker-benchmark: docker-build
	docker run -t imock make benchmark
