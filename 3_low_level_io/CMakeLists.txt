project(low_level_io)
cmake_minimum_required(VERSION 3.10)
 
add_executable(llio main.c)
target_include_directories(llio PRIVATE Include)