CXX = g++
CXXFLAGS = -Wall -std=c++17 -fPIC -I.. -I../common/src
LDFLAGS = -lwebsockets -pthread

# Fuentes para el cliente normal
SRCS = src/main.cpp src/Client.cpp src/ChatUI.cpp src/WebUI.cpp src/WebSocket.cpp src/Message.cpp ../common/src/Protocol.cpp
OBJS = $(SRCS:.cpp=.o)
TARGET = client

# Fuentes para el bot de pruebas
BOT_SRCS = src/ClientBot.cpp src/Client.cpp src/WebSocket.cpp src/Message.cpp ../common/src/Protocol.cpp
BOT_OBJS = $(BOT_SRCS:.cpp=.o)
BOT_TARGET = clientbot

all: $(TARGET) $(BOT_TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(OBJS) -o $(TARGET) $(LDFLAGS)

$(BOT_TARGET): $(BOT_OBJS)
	$(CXX) $(BOT_OBJS) -o $(BOT_TARGET) $(LDFLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(BOT_OBJS) $(TARGET) $(BOT_TARGET)

.PHONY: all clean
