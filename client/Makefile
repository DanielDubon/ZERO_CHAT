CXX = g++
CXXFLAGS = -Wall -std=c++11 -fPIC
INCLUDES = -I.. -I../common/src
LIBS = -lwebsockets -pthread

COMMON_SRCS = ../common/src/Protocol.cpp
CLIENT_SRCS = src/WebSocket.cpp

COMMON_OBJS = $(COMMON_SRCS:.cpp=.o)
CLIENT_OBJS = $(CLIENT_SRCS:.cpp=.o)

TARGETS = test_websocket test_websocket2

.PHONY: all clean

all: $(TARGETS)

test_websocket: $(COMMON_OBJS) $(CLIENT_OBJS) src/test_websocket.o
	$(CXX) $^ -o $@ $(LIBS)

test_websocket2: $(COMMON_OBJS) $(CLIENT_OBJS) src/test_websocket2.o
	$(CXX) $^ -o $@ $(LIBS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -f $(COMMON_OBJS) $(CLIENT_OBJS) src/test_websocket.o src/test_websocket2.o $(TARGETS)
