cmake_minimum_required(VERSION 3.0)

project(julia)

add_executable(julia main.cpp)

install(TARGETS julia RUNTIME DESTINATION bin)
