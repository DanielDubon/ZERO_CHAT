CXX = g++
CXXFLAGS = -Wall -std=c++17 -fPIC -I.. -I../common/src
LDFLAGS = -lwebsockets -pthread

SRCS = src/main.cpp src/Client.cpp src/ChatUI.cpp src/WebUI.cpp src/WebSocket.cpp src/Message.cpp ../common/src/Protocol.cpp
OBJS = $(SRCS:.cpp=.o)
TARGET = client

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(OBJS) -o $(TARGET) $(LDFLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean
