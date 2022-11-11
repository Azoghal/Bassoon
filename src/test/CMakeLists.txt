message(STATUS "attempting to build test executable")

configure_file(test.bs test.bs COPYONLY)

target_sources(Test PUBLIC driver.cxx lexer_test.cxx lexer_test.hxx)
