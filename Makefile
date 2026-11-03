.PHONY: all test clean editor client common server build compile-debug run-tests run-client run-server

HOST ?= 127.0.0.1
PORT ?= 12345

compile-debug:
	cmake -S . -B ./build -DCMAKE_BUILD_TYPE=Debug $(EXTRA_GENERATE)
	cmake --build build/ $(EXTRA_COMPILE)

run-tests: compile-debug
	ctest --test-dir build --output-on-failure

all: run-tests

run-client: compile-debug
	./build/taller_client $(HOST) $(PORT)

run-server: compile-debug
	./build/taller_server $(PORT)

clean:
	rm -Rf build/
