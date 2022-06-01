# Base the image on Ubuntu 22.04.
FROM ubuntu:22.04

# Inform the operating system that no user interaction is possible.
ENV DEBIAN_FRONTEND noninteractive

# Update APT and install necessary packages.
RUN apt update && apt install -y \
    build-essential \
    cmake \
    lcov \
    clang \
    curl

# Download the codecov executable.
RUN curl -Os https://uploader.codecov.io/v0.1.0_4653/linux/codecov

# Make codecov executable.
RUN chmod +x codecov

# Create a directory for the repository.
RUN mkdir /IMock

# Change the directory to the repository directory.
WORKDIR /IMock

# Copy the repository.
COPY . .

# Run make to build the program.
RUN make
