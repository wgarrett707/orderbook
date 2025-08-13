# latest GCC release Docker environment
FROM gcc:latest 

WORKDIR /usr/src/app

COPY ./include/ ./include/
COPY ./src/ ./src/

# Compile C++ program
RUN find ./src -type f -name "*.cpp" ! -name "benchmark.cpp" | xargs g++ -o ./main -I./include

CMD ["./main"]