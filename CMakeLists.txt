project(lsrpc)

cmake_minimum_required(VERSION 3.10)

add_definitions(-D_GLIBCXX_USE_CXX11_ABI=0)

include_directories(include sample)

link_directories(/usr/local/lib)

aux_source_directory(src/ls/rpc SRC)

aux_source_directory(sample SAMPLE)

add_library(lsrpc SHARED  ${SRC})

add_executable(server ${SRC} sample/CASQueueFactory.cpp sample/server.cpp)

add_executable(client sample/client.cpp)

target_link_libraries(server lsexception lscstring lsitem lscopyableitem lslogger lstime lsconfig lsio lsjson lsbuffer lscstring lsepoll lsfile lsnet lshttp pthread)

target_link_libraries(client lsio lsexception lscstring lsitem lscopyableitem lslogger lstime lsconfig lsjson lsbuffer lscstring lsepoll lsfile lsnet lshttp pthread)

install(TARGETS lsrpc DESTINATION lib)

install(DIRECTORY include/ls/rpc DESTINATION include/ls)
