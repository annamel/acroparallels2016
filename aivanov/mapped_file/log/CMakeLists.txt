cmake_minimum_required(VERSION 2.8)
project (log)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -Werror")

add_library(log STATIC log.c logger.c)
target_link_libraries(log pthread rt)

add_executable(server server.c)
target_link_libraries(server log)

add_executable(client client.c)
target_link_libraries(client log)

