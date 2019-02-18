cmake_minimum_required(VERSION 3.13)
project(Shell C)

set(CMAKE_C_STANDARD 99)

add_executable(Shell nush.c svec.c tokens.c)