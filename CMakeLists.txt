cmake_minimum_required(VERSION 3.10)
project(cpp_exam)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(function function.hpp)
add_executable(main main.cpp function)
add_executable(functionTest functionTest.cpp function)

#target_link_libraries(main function)
#target_link_libraries(functionTest function)