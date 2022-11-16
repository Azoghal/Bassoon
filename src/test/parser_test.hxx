#ifndef Bassoon_src_test_parser_test_HXX
#define Bassoon_src_test_parser_test_HXX

namespace bassoon{
namespace test{

int test_parser();

int test_expressions();

int test_bool_expr();
int test_int_expr();
int test_double_expr();
int test_paren_expr();
int test_binop_expr();
int test_call_expr();

int test_statements();

int test_block_st();
int test_call_st();
int test_init_st();
int test_assign_st();
int test_if_st();
int test_for_st();
int test_while_st();
int test_return_st();

int test_definitions();

int test_definition();

} //namespace bassoon
} //namespace test

#endif // Bassoon_src_test_parser_test_HXX