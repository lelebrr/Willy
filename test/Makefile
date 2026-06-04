CXX = g++
CXXFLAGS = -I. -Wall -Wextra -std=c++11 -DWILLY_TESTING

SRCS = test_willy_splash.cpp ../src/ui/willy_config.cpp Arduino.cpp
OBJS = $(SRCS:.cpp=.o)
TARGET = test_runner

.PHONY: all run clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

run: all
	./$(TARGET)
	@rm -f $(OBJS) $(TARGET)

clean:
	rm -f $(OBJS) $(TARGET)
