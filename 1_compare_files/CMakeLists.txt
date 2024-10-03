project(file_diff)
cmake_minimum_required(VERSION 3.10)

set(Sources
	   Src/main.c
	   Src/compare_files.c) 

add_executable(fd ${Sources})
target_include_directories(fd PRIVATE Include)
