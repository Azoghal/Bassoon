#message(STATUS "attempting to build test executable")

file(COPY test_scripts DESTINATION .)
configure_file(test.bs . COPYONLY)
configure_file(brot.bs . COPYONLY)

#target_sources(Test PUBLIC driver.cxx lexer_test.cxx lexer_test.hxx parser_test.cxx parser_test.hxx)
target_sources(Test PUBLIC driver.cxx lexer_test.cxx parser_test.cxx)
