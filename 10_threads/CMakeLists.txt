project(threads_lab)

cmake_minimum_required(VERSION 3.10)

add_executable (t1 Src/experiment_tmm_pt1.c
		   Src/mat_mul_pt1.c
		   Src/tspec_diff.c)

target_include_directories(t1 PRIVATE Include)

add_executable (t2 Src/experiment_tmm_pt2.c
		   Src/mat_mul_pt2.c
		   Src/tspec_diff.c)

target_include_directories(t2 PRIVATE Include)

