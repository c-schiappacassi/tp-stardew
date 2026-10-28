.PHONY: all test clean editor client common server build

compile-debug:
	mkdir -p build/
	cmake -S . -B ./build -DCMAKE_BUILD_TYPE=Debug $(EXTRA_GENERATE)
	cmake --build  build/ $(EXTRA_COMPILE)

run-tests: compile-debug
	./build/taller_tests

all: clean run-tests

run-client: compile-debug
	./build/taller_client

run-server: compile-debug
	./build/taller_server
	
clean:
	rm -Rf build/
