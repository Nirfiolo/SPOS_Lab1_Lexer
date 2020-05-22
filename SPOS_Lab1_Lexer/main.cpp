#include "lexer.h"

#include <iostream>


int main()
{
    lexer::lexer_output_t const lexer_output = lexer::get_tokens("code.txt");

    lexer::output_lexer_data(std::cout, lexer_output);
}