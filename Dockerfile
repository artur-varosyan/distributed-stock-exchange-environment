# Run on Amazon Linux 2023
FROM amazonlinux:2023

# Copy source files
COPY CMakeLists.txt /app/CMakeLists.txt
COPY src /app/src

# Set container working directory
WORKDIR /app

# Install g++ and cmake
RUN yum install -y gcc-c++ cmake

# Install the boost library
RUN yum install -y boost-devel

# Build the project
RUN cmake -B build

# Compile the project
RUN cd build && make


