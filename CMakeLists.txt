cmake_minimum_required(VERSION 3.21)
project(CMinusCompiler C)

set(CMAKE_C_STANDARD 17)

add_executable(CMinusCompiler main.c globals.h scanner.h parser.h scanner.c parser.c utility.h utility.c code.h code.c)
