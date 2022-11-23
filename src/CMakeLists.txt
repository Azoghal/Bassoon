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

####separate_arguments(LLVM_DEFINITIONS_LIST NATIVE_COMMAND ${LLVM_DEFINITIONS})
add_definitions(${LLVM_DEFINITIONS_LIST})

add_executable(Bassoon bassoon.cxx lexer.cxx parser.cxx codegen.cxx viz_visitor.cxx)

#if(testing_enabled)
add_executable(Test lexer.cxx parser.cxx ast.cxx codegen.cxx)
add_subdirectory(test)
#endif();

llvm_map_components_to_libnames(llvm_libs core support irreader orcjit native)

target_link_libraries(Bassoon ${llvm_libs})
