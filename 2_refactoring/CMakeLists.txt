project(refactoring_practice)
cmake_minimum_required(VERSION 3.10)

add_executable(perm Src/perm.c)

add_executable(permute Src/permute.c
		       Src/gen_perms.c)

target_include_directories(permute PRIVATE Include)