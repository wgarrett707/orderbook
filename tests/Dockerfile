# Start with the latest GCC release Docker environment
FROM gcc:latest

# Install dependencies for Google Test and cmake
RUN apt-get update && apt-get install -y cmake git build-essential

# Set the working directory for the project
WORKDIR /usr/src/project

# Copy your project files into the container
COPY . /usr/src/project

# Build and install Google Test
RUN git clone https://github.com/google/googletest.git /usr/src/googletest \
    && mkdir /usr/src/googletest/build \
    && cd /usr/src/googletest/build \
    && cmake .. \
    && make \
    && make install

# Set the working directory for the tests
WORKDIR /usr/src/project/tests

# Compile the C++ test suite, including all source files
RUN g++ -o tests tests.cpp ../src/*.cpp -I../include -lgtest -lgtest_main -pthread

# Run the tests
CMD ["./tests"]
