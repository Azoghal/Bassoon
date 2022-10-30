cmake_minimum_required(VERSION 3.16)
set(CMAKE_CXX_STANDARD 17)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_C_COMPILER clang)
set(CMAKE_CXX_COMPILER clang++)

project(Bassoon VERSION 1.0 LANGUAGES C CXX)

add_subdirectory(src)

find_package(LLVM REQUIRED CONFIG)

link_libraries()
execute_process(COMMAND llvm-config --libs OUTPUT_VARIABLE LIBS)
execute_process(COMMAND llvm-config --system-libs OUTPUT_VARIABLE SYS_LIBS)
execute_process(COMMAND llvm-config --ldflags OUTPUT_VARIABLE LDF)
message(STATUS "Found LLVM" ${LIBS})

string(STRIP ${LIBS} LIBS)
string(STRIP ${SYS_LIBS} SYS_LIBS)
string(STRIP ${LDF} LDF)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")


link_libraries(${LIBS} ${SYS_LIBS} ${LDF})

# Enable project compile flags.. 

include_directories(${LLVM_INCLUDE_DIRS})
#separate_arguments(LLVM_DEFINITIONS_LIST NATIVE_COMMAND ${LLVM_DEFINITIONS})
add_definitions(${LLVM_DEFINITIONS_LIST})

add_executable(Bassoon src/bassoon.cxx src/lexer.cxx)
add_executable(Test src/test/driver.cxx src/test/lexer_test.cxx src/lexer.cxx)

llvm_map_components_to_libnames(llvm_libs core support irreader orcjit native)

target_link_libraries(Bassoon ${llvm_libs})

#configure_file(source_name destination_name COPYONLY)

#target_include_directories(Bassoon PUBLIC)

#install(TARGETS Bassoon DESTINATION bin)