# Run on Alpine
FROM alpine:latest

# Copy source files
COPY CMakeLists.txt /app/CMakeLists.txt
COPY src /app/src

# Set container working directory
WORKDIR /app

# Install g++ and cmake
RUN apk add build-base cmake

# Install the boost library
RUN apk add boost-dev

# Install the boost library
# RUN yum install -y wget tar bzip2
# RUN wget https://boostorg.jfrog.io/artifactory/main/release/1.84.0/source/boost_1_84_0.tar.bz2
# RUN ls -al
# RUN tar --bzip2 -xf boost_1_84_0.tar.bz2
# RUN cd boost_1_84_0 && ./bootstrap.sh && ./b2 install

# Build the project
RUN cmake -B build

# Compile the project
RUN cd build && make


