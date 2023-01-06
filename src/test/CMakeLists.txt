message(STATUS "attempting to build test executable")

file(COPY test_scripts DESTINATION .)
file(COPY test.bs DESTINATION .)
#target_sources(Test PUBLIC driver.cxx lexer_test.cxx lexer_test.hxx parser_test.cxx parser_test.hxx)
target_sources(Test PUBLIC driver.cxx lexer_test.cxx parser_test.cxx)
