CXX = g++
CXXFLAGS = -Wall -std=c++11
INCLUDES = -Isrc

SRCS = src/Protocol.cpp
OBJS = $(SRCS:.cpp=.o)
TARGET = libprotocol.a

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJS)
	ar rcs $(TARGET) $(OBJS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)
