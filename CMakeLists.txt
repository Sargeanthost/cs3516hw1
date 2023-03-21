cmake_minimum_required(VERSION 3.25)
project(proj1 C)

set(CMAKE_C_STANDARD 11)

add_executable(proj1 client/http_client.c)
