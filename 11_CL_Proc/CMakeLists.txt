cmake_minimum_required(VERSION 3.22)
project(11_CL_Proc C)

set(CMAKE_C_STANDARD 11)

add_executable(cmakepart1
        lander_pt1.c)

add_executable(cmakepart2
        lander_pt2.c)

add_executable(cmakepart2
        lander_pt3.c)

target_link_libraries(cmakepart1 PRIVATE m)
target_link_libraries(cmakepart2 PRIVATE m)
target_link_libraries(cmakepart3 PRIVATE m)
