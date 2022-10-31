message(STATUS "attempting to build test executable")

target_sources(Test PUBLIC driver.cxx lexer_test.cxx lexer_test.hxx)
