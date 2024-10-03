cmake_minimum_required(VERSION 3.21)
project(part1 C)

add_executable(part1 Src/plotz2_pt1.c)
add_executable(part2 Src/plotz2_pt2.c)
add_executable(part3 Src/plotz2_pt3.c)
add_executable(part4 Src/plotz2_pt4.c)

target_link_libraries(part1 PRIVATE m)
target_link_libraries(part2 PRIVATE m)
target_link_libraries(part3 PRIVATE m)
target_link_libraries(part4 PRIVATE m)

target_include_directories(part1 PRIVATE Include)
target_include_directories(part2 PRIVATE Include)
target_include_directories(part3 PRIVATE Include)
target_include_directories(part4 PRIVATE Include)

