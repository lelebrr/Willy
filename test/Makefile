CXX = g++
CXXFLAGS = -I. -std=c++11

all: test_runner

setup_gpio_impl.cpp: ../src/main.cpp
	python3 extract_test.py

test_runner: test_main_setup_gpio.cpp setup_gpio_impl.cpp
	$(CXX) $(CXXFLAGS) test_main_setup_gpio.cpp -o $@

clean:
	rm -f test_runner setup_gpio_impl.cpp
