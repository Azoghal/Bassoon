cmake_minimum_required(VERSION 3.16)
set(CMAKE_CXX_STANDARD 20)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_C_COMPILER clang)
set(CMAKE_CXX_COMPILER clang++)

project(Bassoon VERSION 1.0 LANGUAGES C CXX)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

include_directories(include)

add_subdirectory(src)

make_directory(out)

#add_executable(bassoon src/bassoon.cxx src/lexer.cxx src/parser.cxx)

#configure_file(source_name destination_name COPYONLY)

#target_include_directories(Bassoon PUBLIC)

#install(TARGETS Bassoon DESTINATION bin)