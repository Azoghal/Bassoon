#ifndef Bassoon_src_test_lexer_HXX
#define Bassoon_src_test_lexer_HXX

namespace bassoon{
namespace test{

int test_lexer();

int test_immediate_int();
int test_immediate_double();
int test_immediate_true();
int test_immediate_false();

int test_function_def();
int test_typed_variables();

} //namespace bassoon
} //namespace test

#endif // Bassoon_src_test_lexer_HXX