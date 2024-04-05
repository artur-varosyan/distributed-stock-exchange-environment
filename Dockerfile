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

# Build the project
RUN cmake -B build

# Compile the project
RUN cd build && make

WORKDIR /app/build

ENTRYPOINT [ "./simulation" ]


