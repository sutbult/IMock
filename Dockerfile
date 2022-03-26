# Base the image on Ubuntu 20.04.
FROM ubuntu:20.04

# Inform the operating system that no user interaction is possible.
ENV DEBIAN_FRONTEND noninteractive

# Update APT and install packages necessary to build the program.
RUN apt update && apt install -y \
    build-essential \
    python3-pip \
    cmake \
    lcov

# Create a directory for the repository.
RUN mkdir /IMock

# Change the directory to the repository directory.
WORKDIR /IMock

# Copy the repository.
COPY . .

# Run make to build the program.
RUN make
