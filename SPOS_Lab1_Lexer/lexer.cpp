#include "lexer.h"

#include <fstream>
#include <cassert>
#include <iomanip>
#include <algorithm>


namespace lexer
{
    bool is_space(char c) noexcept
    {
        return std::isspace(static_cast<unsigned char>(c));
    }

    bool is_digit(char c) noexcept
    {
        return std::isdigit(static_cast<unsigned char>(c));
    }

    bool is_alpha(char c) noexcept
    {
        return std::isalpha(static_cast<unsigned char>(c));
    }

    bool is_alpha_or_digit(char c) noexcept
    {
        return is_alpha(c) || is_digit(c);
    }

    bool is_valid_word_begin(char c) noexcept
    {
        return is_alpha(c) || (c == '_');
    }
    bool is_valid_word_part(char c) noexcept
    {
        return is_alpha_or_digit(c) || (c == '_');
    }

    bool is_operator(char c) noexcept
    {
        switch (c)
        {
        case '=':
        case '+':
        case '-':
        case '*':
        case '/':
        case '%':
        case '&':
        case '|':
        case '!':
        case '<':
        case '>':
        case '~':
        case '^':
        case '?':
        case ':':
            return true;
        default:
            return false;
        }
    }

    bool is_valid_number_begin(char c) noexcept
    {
        return is_digit(c) || (c == '.');
    }
    bool is_valid_number_part(char c) noexcept
    {
        return is_digit(c) || (c == '.') || (c == '\'') || (c == 'e');
    }

    // TODO: hax and bool number

    bool is_valid_symbol_after_number(char c) noexcept
    {
        // TODO: punctuation marks
        return is_operator(c) || is_space(c);
    }

    bool is_symbol_type(TokenType type) noexcept
    {
        switch (type)
        {
        case lexer::TokenType::IntNumber:
        case lexer::TokenType::FloatNumber:
        case lexer::TokenType::Id:
            return true;
        default:
            return false;
        }
    }



    struct FANode
    {
        char c{ '\0' };
        TokenType type{ TokenType::Invalid };

        std::vector<FANode> children{};
    };

    static FANode fa_root{};

    void generate_fa_node(
        std::vector<std::pair<std::string, TokenType>> const & string_to_type,
        FANode & node,
        size_t offset,
        size_t & position
    ) noexcept
    {
        do
        {
            size_t const last_poistion = position;
            if (string_to_type[position].first.size() > offset)
            {
                FANode new_node{};
                generate_fa_node(string_to_type, new_node, offset + 1, position);
                node.children.emplace_back(std::move(new_node));
            }
            else
            {
                node.c = string_to_type[position].first[offset - 1];
                node.type = string_to_type[position].second;
                ++position;
            }

            for (size_t i = 0; i < offset && position < string_to_type.size(); ++i)
            {
                if (string_to_type[position].first.size() <= i || string_to_type[position].first[i] != string_to_type[last_poistion].first[i])
                {
                    return;
                }
            }

        } while (position < string_to_type.size());
    }

    void generate_fa() noexcept
    {
        constexpr size_t begin = static_cast<size_t>(TokenType::OperatorsBegin) + 1;
        constexpr size_t end = static_cast<size_t>(TokenType::OperatorsEnd);

        std::vector<std::pair<std::string, TokenType>> string_to_type(end - begin);

        for (size_t i = begin; i < end; ++i)
        {
            string_to_type[i - begin] = { Token_to_string[i],  static_cast<TokenType>(i) };
        }

        std::sort(string_to_type.begin(), string_to_type.end());
        size_t position = 0;
        generate_fa_node(string_to_type, fa_root, 0, position);
    }




    std::pair<TokenType, bool> try_get_keywords(std::string_view word) noexcept
    {
        for (size_t i = static_cast<size_t>(TokenType::KeywordsBegin) + 1; i < static_cast<size_t>(TokenType::KeywordsEnd); ++i)
        {
            if (word == std::string_view{ Token_to_string[i] })
            {
                return { static_cast<TokenType>(i), true };
            }
        }
        return { TokenType::Invalid, false };
    }

    std::pair<size_t, bool> try_get_from_symbol_table(symbol_table_t const & symbol_table, std::string const & symbol) noexcept
    {
        // TODO:

        return { std::numeric_limits<size_t>::max(), false };
    }

    void create_new_token(
        symbol_table_t & symbol_table,
        tokens_t & tokens,
        size_t line,
        size_t column,
        TokenType type,
        std::string_view symbol = ""
    ) noexcept
    {
        if (is_symbol_type(type))
        {
            tokens.push_back({ line, column, type, symbol_table.size() });
            symbol_table.push_back(std::string{ symbol });
        }
        else
        {
            tokens.push_back({ line, column, type });
        }
    }

    void handle_operator_by_fa(
        symbol_table_t & symbol_table,
        tokens_t & tokens,
        token_errors_t & token_errors,
        std::string_view code,
        size_t line,
        size_t & column,
        size_t start,
        FANode const & node
    ) noexcept
    {
        char const current_char = code[column];
        bool const is_current_char_operator = is_operator(current_char);

        if (!is_current_char_operator)
        {
            create_new_token(symbol_table, tokens, line, start, node.type);
            return;
        }

        ++column;
        for (size_t i = 0; i < node.children.size(); ++i)
        {
            if (node.children[i].c == current_char)
            {
                handle_operator_by_fa(symbol_table, tokens, token_errors, code, line, column, column, node.children[i]);
                return;
            }
        }

        // TODO: error
    }

    void handle_operator_by_fa(
        symbol_table_t & symbol_table,
        tokens_t & tokens,
        token_errors_t & token_errors,
        std::string_view code,
        size_t line,
        size_t & column
    ) noexcept
    {
        handle_operator_by_fa(symbol_table, tokens, token_errors, code, line, column, column, fa_root);
    }

    void handle_digit(symbol_table_t & symbol_table, tokens_t & tokens, token_errors_t & token_errors, std::string_view code, size_t line, size_t & column) noexcept
    {
        // TODO: handle single litter symbol after number

        char c = code[column];

        // TODO: 
        bool has_dot = false;
        bool has_e = false;
        bool has_x = false;
        bool has_b = false;

        size_t const start = column;
        ++column;
        while (column < code.size() && is_valid_number_part(code[column]))
        {
            ++column;
        }
        std::string_view number = code.substr(start, column - start);

        if (column < code.size() && !is_valid_symbol_after_number(code[column]))
        {
            // TODO: error
        }

        if (has_dot)
        {
            create_new_token(symbol_table, tokens, line, start, TokenType::FloatNumber, number);
        }
        if (!has_dot && !has_x && !has_b)
        {
            create_new_token(symbol_table, tokens, line, start, TokenType::IntNumber, number);
        }
    }

    bool next_token(symbol_table_t & symbol_table, tokens_t & tokens, token_errors_t & token_errors, std::string_view code, size_t line, size_t & column, bool & is_now_commented_code) noexcept
    {
        while (column < code.size() && is_space(code[column]))
        {
            ++column;
        }

        if (column == code.size())
        {
            return false;
        }

        char c = code[column];


        if (is_valid_number_begin(c))
        {
            handle_digit(symbol_table, tokens, token_errors, code, line, column);
            return true;
        }
        // TODO: is_symbolic_constants
        // TODO: is_preprocessor_directives
        // TODO: is_comments
        // TODO: is_word
        if (is_operator(c))
        {
            handle_operator_by_fa(symbol_table, tokens, token_errors, code, line, column);
            return true;
        }
        // TODO: is_punctuation_marks


        ++column;
        // TODO: make error
        return true;
    }



    lexer_output_t get_tokens(std::string const & file_path) noexcept(!IS_DEBUG)
    {
        std::ifstream file_input{ file_path };

        if (!file_input)
        {
            assert(false && "Cannot open file");
            return {};
        }

        if (fa_root.children.empty())
        {
            generate_fa();
        }

        symbol_table_t symbol_table{};
        tokens_t tokens{};
        token_errors_t token_errors{};

        bool is_now_commented_code = false;

        std::string code;

        size_t line = 0;
        while (std::getline(file_input, code))
        {
            size_t column = 0;
            while (next_token(symbol_table, tokens, token_errors, code, line, column, is_now_commented_code))
            {

            }
            ++line;
        }

        return { symbol_table, { tokens, token_errors } };
    }

    void output_lexer_data(std::ostream & os, lexer_output_t const & lexer_output) noexcept
    {
        lexer::symbol_table_t const & symbol_table = lexer_output.first;
        lexer::tokens_t const & tokens = lexer_output.second.first;
        lexer::token_errors_t const & token_errors = lexer_output.second.second;

        if (token_errors.empty())
        {
            os << "No errors\n\n";
        }
        else
        {
            os << "Error: " << token_errors.size() << '\n';
            for (size_t i = 0; i < token_errors.size(); ++i)
            {
                os << "Line: " << std::right << std::setw(4) << token_errors[i].line <<
                    '[' << std::left << std::setw(4) << token_errors[i].column << ']' << ' ';
                os << token_errors[i].message;
                os << '\n';
            }
            os << '\n';
        }

        os << "Tokens:\n";
        for (size_t i = 0; i < tokens.size(); ++i)
        {
            os << std::left;
            os << "Id: " << std::setw(3) << i << ' ';
            os << "Type: " << std::setw(15) << Token_to_string[static_cast<size_t>(tokens[i].type)] << ' ';
            os << "Line: " << std::right << std::setw(4) << tokens[i].line <<
                '[' << std::left << std::setw(4) << tokens[i].column << ']' << ' ';
            os << "Symbol: " << std::setw(0) << (is_symbol_type(tokens[i].type) ? symbol_table[tokens[i].index_in_symbol_table] : "");
            os << '\n';
        }
    }
}