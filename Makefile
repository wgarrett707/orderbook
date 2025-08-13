CXXFLAGS= -std=c++17 -Iinclude
CXX = g++
OBJS = $(SRCS:.cpp=.o)

clean:
	rm -f bin/*

exec: ./src/main.cpp ./include/Orderbook.h ./include/OrderTypes.h ./include/Order.h ./include/types.h
	$(CXX) $(CXXFLAGS) ./src/main.cpp ./src/Orderbook.cpp ./src/Order.cpp ./src/OrderTypes.cpp -o bin/exec

tests: ./tests/tests.cpp ./include/Orderbook.h ./include/OrderTypes.h ./include/Order.h
	$(CXX) $(CXXFLAGS) ./tests/tests.cpp ./src/Orderbook.cpp -o bin/exec-tests

benchmark: ./src/benchmark.cpp ./include/Orderbook.h ./include/OrderTypes.h ./include/Order.h ./include/OrderGenerator.h
	$(CXX) $(CXXFLAGS) ./src/benchmark.cpp ./src/Orderbook.cpp ./src/OrderGenerator.cpp -o bin/exec-benchmark

src/%.cc: includes/%.hpp
	touch $@

server: $(OBJS)
	$(CXX) $(CXXFLAGS) server-windows.cpp src/Orderbook.cpp src/Order.cpp src/OrderTypes.cpp -o bin/server $(OBJS)

.DEFAULT_GOAL := exec