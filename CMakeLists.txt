cmake_minimum_required(VERSION 3.10)
project(ZeroChat)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Directorios de inclusión
include_directories(
    ${PROJECT_SOURCE_DIR}/common/src
    ${PROJECT_SOURCE_DIR}/client/src
    ${PROJECT_SOURCE_DIR}/server/src
)

# Cliente
add_executable(client
    client/src/main.cpp
    client/src/Client.cpp
    client/src/UI.cpp
    client/src/WebSocket.cpp
    common/src/Protocol.cpp
)

# Servidor (opcional, si lo necesitas)
add_executable(server
    server/src/main.cpp
    server/src/Server.cpp
    server/src/User.cpp
    server/src/Message.cpp
    common/src/Protocol.cpp
)

# Buscar y enlazar libwebsockets
find_package(LibWebsockets REQUIRED)
target_link_libraries(client websockets pthread)
target_link_libraries(server websockets pthread) 