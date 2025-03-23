# Ejemplo de Makefile en server/Makefile

CXX = g++
CXXFLAGS = -Wall -std=c++11
INCLUDES = -I../common/src -Isrc
LIBS = -lwebsockets -pthread

SRCS = src/main.cpp \
       src/Server.cpp \
       src/Message.cpp \
       src/User.cpp \
       src/WebSocket.cpp \
       ../common/src/Protocol.cpp

OBJS = $(SRCS:.cpp=.o)
TARGET = server

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(OBJS) -o $(TARGET) $(LIBS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)
