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

    bool is_lower(char c) noexcept
    {
        return std::islower(static_cast<unsigned char>(c));
    }

    bool is_upper(char c) noexcept
    {
        return std::isupper(static_cast<unsigned char>(c));
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
        case '.':
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
        return is_digit(c) || (c == '.') || (c == '\'');
    }

    bool is_binary_number(char c) noexcept
    {
        return (c == '0' || c == '1');
    }

    bool is_valid_binary_number_part(char c) noexcept
    {
        return (is_binary_number(c) || c == '\'');
    }

    bool is_hex_number(char c) noexcept
    {
        return std::isxdigit(static_cast<unsigned char>(c));
    }

    bool is_valid_hex_number_part(char c) noexcept
    {
        return (is_hex_number(c) || c == '\'');
    }

    bool is_punctuation_marks(char c) noexcept
    {
        switch (c)
        {
        case ',':
        case ';':
        case '(':
        case ')':
        case '[':
        case ']':
        case '{':
        case '}':
            return true;
        default:
            return false;
        }
    }

    bool is_valid_symbol_after_number(char c) noexcept
    {
        return is_operator(c) || is_space(c) || is_punctuation_marks(c) || c == '/';
    }

    bool is_symbol_type(TokenType type) noexcept
    {
        switch (type)
        {
        case lexer::TokenType::IntNumber:
        case lexer::TokenType::FloatNumber:
        case lexer::TokenType::Character:
        case lexer::TokenType::String:
        case lexer::TokenType::SingleLineComment:
        case lexer::TokenType::MultyLineComment:
        case lexer::TokenType::Id:
            return true;
        default:
            return false;
        }
    }

    bool is_multi_line_preprocessor_directives(TokenType type) noexcept
    {
        switch (type)
        {
        case lexer::TokenType::SharpIf:
        case lexer::TokenType::SharpIfdef:
        case lexer::TokenType::SharpIfndef:
        case lexer::TokenType::SharpElif:
        case lexer::TokenType::SharpElse:
        case lexer::TokenType::SharpEndif:
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
                if (string_to_type[position].first.size() <= i ||
                    string_to_type[position].first[i] != string_to_type[last_poistion].first[i])
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


    std::pair<TokenType, bool> try_get_preprocessor_directives(std::string_view word) noexcept
    {
        for (
            size_t i = static_cast<size_t>(TokenType::PreprocessorDirectivesBegin) + 1;
            i < static_cast<size_t>(TokenType::PreprocessorDirectivesEnd);
            ++i
            )
        {
            if (word == std::string_view{ Token_to_string[i] })
            {
                return { static_cast<TokenType>(i), true };
            }
        }
        return { TokenType::Invalid, false };
    }

    std::pair<TokenType, bool> try_get_keywords(std::string_view word) noexcept
    {
        for (
            size_t i = static_cast<size_t>(TokenType::KeywordsBegin) + 1;
            i < static_cast<size_t>(TokenType::KeywordsEnd);
            ++i
            )
        {
            if (word == std::string_view{ Token_to_string[i] })
            {
                return { static_cast<TokenType>(i), true };
            }
        }
        return { TokenType::Invalid, false };
    }

    std::pair<size_t, bool> try_get_from_symbol_table(symbol_table_t const & symbol_table, std::string_view symbol) noexcept
    {
        for (size_t i = 0; i < symbol_table.size(); ++i)
        {
            if (symbol == symbol_table[i])
            {
                return { i, true };
            }
        }

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
            std::pair<size_t, bool> const from_symbol_table = try_get_from_symbol_table(symbol_table, symbol);
            if (from_symbol_table.second)
            {
                tokens.push_back({ line, column, type, from_symbol_table.first });
            }
            else
            {
                tokens.push_back({ line, column, type, symbol_table.size() });
                symbol_table.push_back(std::string{ symbol });
            }
        }
        else
        {
            tokens.push_back({ line, column, type });
        }
    }

    void create_new_token_error(
        token_errors_t & token_errors,
        std::string const & message,
        std::string const & symbol,
        size_t line,
        size_t column
    ) noexcept
    {
        size_t const length = symbol.size();

        if (!token_errors.empty())
        {
            TokenError & last_token_error = token_errors.back();

            if (last_token_error.line == line &&
                last_token_error.column + last_token_error.length == column &&
                last_token_error.message == message
                )
            {
                last_token_error.length += length;
                last_token_error.symbol += symbol;
                return;
            }
        }
        token_errors.push_back({ message, symbol, line, column, length });
    }

    void create_new_token_error(
        token_errors_t & token_errors,
        std::string const & symbol,
        size_t line,
        size_t column
    ) noexcept
    {
        create_new_token_error(token_errors, { "Error: \"token\" could be introduced" }, symbol, line, column);
    }

    void handle_operator_by_fa(
        symbol_table_t & symbol_table,
        tokens_t & tokens,
        token_errors_t & token_errors,
        std::string_view code,
        size_t line,
        size_t & column
    ) noexcept;

    void handle_digit(
        symbol_table_t & symbol_table,
        tokens_t & tokens,
        token_errors_t & token_errors,
        std::string_view code,
        size_t line,
        size_t & column
    ) noexcept
    {
        char const c = code[column];

        size_t const start = column;
        ++column;

        bool has_dot = (c == '.');
        size_t dot_position = (has_dot ? column : std::numeric_limits<size_t>::max());
        bool is_decimal = true;
        bool is_hex = false;
        bool is_binary = false;
        bool const is_first_zero = (c == '0');

        size_t last_number_separator_index = start;

        if (column >= code.size())
        {
            if (has_dot)
            {
                --column;
                handle_operator_by_fa(symbol_table, tokens, token_errors, code, line, column);
                return;
            }
            create_new_token(symbol_table, tokens, line, start, TokenType::IntNumber, code.substr(start, 1));
            return;
        }

        char const next_char = code[column];
        if (has_dot && !is_digit(next_char))
        {
            --column;
            handle_operator_by_fa(symbol_table, tokens, token_errors, code, line, column);
            return;
        }
        if (!is_first_zero && !has_dot && !is_valid_number_begin(next_char))
        {
            create_new_token(symbol_table, tokens, line, start, TokenType::IntNumber, code.substr(start, 1));
            return;
        }
        if (is_first_zero && next_char == 'b')
        {
            is_binary = true;
            is_decimal = false;
        }
        else if (is_first_zero && next_char == 'x')
        {
            is_hex = true;
            is_decimal = false;
        }
        else if (!is_valid_number_part(next_char))
        {
            create_new_token(symbol_table, tokens, line, start, TokenType::IntNumber, code.substr(start, 1));
            return;
        }

        if (next_char == '\'')
        {
            last_number_separator_index = column;
        }

        if (next_char == '.')
        {
            has_dot = true;
            dot_position = column;
        }

        ++column;

        while (column < code.size() &&
            ((is_decimal && is_valid_number_part(code[column])) ||
                (is_hex && is_valid_hex_number_part(code[column])) ||
                (is_binary && is_valid_binary_number_part(code[column]))))
        {
            if (has_dot && code[column] == '.')
            {
                ++column;
                create_new_token_error(
                    token_errors,
                    "Error: double dot in number value",
                    std::string{ code.substr(start, column - start) },
                    line,
                    start
                );
                return;
            }
            if (!has_dot && code[column] == '.')
            {
                if (column - last_number_separator_index == 1)
                {
                    ++column;
                    create_new_token_error(
                        token_errors,
                        "Error: number separator and dot too close",
                        std::string{ code.substr(start, column - start) },
                        line,
                        start
                    );
                    return;
                }
                has_dot = true;
                dot_position = column;
            }
            if (code[column] == '\'')
            {
                if (column - last_number_separator_index == 1)
                {
                    ++column;
                    create_new_token_error(
                        token_errors,
                        "Error: number separators too close",
                        std::string{ code.substr(start, column - start) },
                        line,
                        start
                    );
                    return;
                }
                if (column - dot_position == 1)
                {
                    ++column;
                    create_new_token_error(
                        token_errors,
                        "Error: dot and number separator too close",
                        std::string{ code.substr(start, column - start) },
                        line,
                        start
                    );
                    return;
                }
                last_number_separator_index = column;
            }
            ++column;
        }

        if (column < code.size() && !is_valid_symbol_after_number(code[column]))
        {
            ++column;
            create_new_token_error(
                token_errors,
                "Error: invalid symbol after number",
                std::string{ code.substr(start, column - start) },
                line,
                start
            );
            return;
        }
        if (!((is_decimal && is_digit(code[column - 1])) ||
            (is_hex && is_hex_number(code[column - 1])) ||
            (is_binary && is_binary_number(code[column - 1]))))
        {
            create_new_token_error(
                token_errors,
                "Error: invalid number end",
                std::string{ code.substr(start, column - start) },
                line,
                start
            );
            return;
        }

        std::string_view const number = code.substr(start, column - start);
        if (has_dot)
        {
            create_new_token(symbol_table, tokens, line, start, TokenType::FloatNumber, number);
        }
        if (!has_dot)
        {
            create_new_token(symbol_table, tokens, line, start, TokenType::IntNumber, number);
        }
    }

    void handle_literals_constant(
        symbol_table_t & symbol_table,
        tokens_t & tokens,
        token_errors_t & token_errors,
        std::string_view code,
        size_t line,
        size_t & column
    ) noexcept
    {
        char const c = code[column];

        size_t const start = column;
        ++column;
        if (column >= code.size())
        {
            create_new_token_error(
                token_errors,
                "Error: unfinished symbol: symbol on end of line",
                std::string{ c },
                line,
                column
            );
            return;
        }
        char next_char = code[column];

        if (next_char == '\'')
        {
            create_new_token_error(
                token_errors,
                "Error: empty character constant",
                std::string{ c, next_char },
                line,
                column
            );
            return;
        }

        bool const is_need_additional_char = (next_char == '\\');
        char additional_char;
        if (is_need_additional_char)
        {
            ++column;
            additional_char = next_char;

            if (column >= code.size())
            {
                create_new_token_error(
                    token_errors,
                    "Error: unfinished symbol: symbols on end of line",
                    std::string{ c, additional_char },
                    line,
                    column
                );
                return;
            }
            next_char = code[column];
        }

        ++column;
        if (column >= code.size())
        {
            if (is_need_additional_char)
            {
                create_new_token_error(
                    token_errors,
                    "Error: unfinished symbol: symbols on end of line",
                    std::string{ c, additional_char, next_char },
                    line,
                    column
                );
                return;
            }
            create_new_token_error(
                token_errors,
                "Error: unfinished symbol: symbols on end of line",
                std::string{ c, next_char },
                line,
                column
            );
            return;
        }
        char const last_char = code[column];

        if (last_char != '\'')
        {
            if (is_need_additional_char)
            {
                create_new_token_error(
                    token_errors,
                    "Error: too many characters in symbol constant",
                    std::string{ c, additional_char, next_char, last_char },
                    line,
                    column
                );
                return;
            }
            create_new_token_error(
                token_errors,
                "Error: too many characters in symbol constant",
                std::string{ c, next_char, last_char },
                line,
                column
            );
            return;
        }
        ++column;

        std::string_view const word = code.substr(start, column - start);

        create_new_token(symbol_table, tokens, line, start, TokenType::Character, word);
    }

    struct BetweenLinesData
    {
        std::string data{ "" };
        size_t line;
        size_t column;
        bool is_active{ false };
        // Need for comments, string_constant ignore this field
        // true - comment like: // ...
        // false - comment like: /* ... */
        bool is_first_comment_type{ false };
    };

    void handle_string_constant(
        symbol_table_t & symbol_table,
        tokens_t & tokens,
        token_errors_t & token_errors,
        std::string_view code,
        size_t line,
        size_t & column,
        BetweenLinesData & string_constant_data
    ) noexcept
    {
        size_t const start = column;
        if (!string_constant_data.is_active)
        {
            ++column;
        }

        bool is_previous_spesial_symbol = false;

        while (column < code.size() && !(!is_previous_spesial_symbol && code[column] == '\"'))
        {
            if (is_previous_spesial_symbol)
            {
                is_previous_spesial_symbol = false;
            }
            else if (!is_previous_spesial_symbol && code[column] == '\\')
            {
                is_previous_spesial_symbol = true;
            }
            ++column;
        }

        if (column >= code.size() && is_previous_spesial_symbol)
        {
            if (string_constant_data.is_active)
            {
                string_constant_data.data += std::string{ code.substr(start, column - start - 1) };
                return;
            }
            string_constant_data.data = std::string{ code.substr(start, column - start - 1) };
            string_constant_data.line = line;
            string_constant_data.column = start;
            string_constant_data.is_active = true;
            return;
        }
        else if (column >= code.size())
        {
            if (string_constant_data.is_active)
            {
                string_constant_data.data += std::string{ code.substr(start, column - start) };
                create_new_token_error(
                    token_errors,
                    "Error: unfinished string constant",
                    string_constant_data.data,
                    string_constant_data.line,
                    string_constant_data.column
                );
                string_constant_data.is_active = false;
                return;
            }
            create_new_token_error(
                token_errors,
                "Error: unfinished string constant",
                std::string{ code.substr(start, column - start) },
                line,
                column
            );
            return;
        }

        ++column;
        std::string_view const word = code.substr(start, column - start);

        if (string_constant_data.is_active)
        {
            string_constant_data.data += std::string{ word };
            create_new_token(
                symbol_table,
                tokens,
                string_constant_data.line,
                string_constant_data.column,
                TokenType::String,
                string_constant_data.data
            );
            string_constant_data.is_active = false;
            return;
        }

        create_new_token(symbol_table, tokens, line, start, TokenType::String, word);
    }

    void handle_preprocessor_directives(
        symbol_table_t & symbol_table,
        tokens_t & tokens,
        token_errors_t & token_errors,
        std::string_view code,
        size_t line,
        size_t & column,
        bool & is_now_preprocessor_directives
    ) noexcept
    {
        size_t const start = column;
        ++column;
        while (column < code.size() && is_lower(code[column]))
        {
            ++column;
        }

        std::string_view const word = code.substr(start, column - start);

        std::pair<TokenType, bool> const preprocessor_directives = try_get_preprocessor_directives(word);
        if (!preprocessor_directives.second)
        {
            create_new_token_error(
                token_errors,
                "Error: undefined preprocessor directives",
                std::string{ word },
                line,
                column
            );
            return;
        }

        create_new_token(symbol_table, tokens, line, start, preprocessor_directives.first);
        if (!is_multi_line_preprocessor_directives(preprocessor_directives.first))
        {
            is_now_preprocessor_directives = true;
        }
    }

    void handle_comments(
        symbol_table_t & symbol_table,
        tokens_t & tokens,
        token_errors_t & token_errors,
        std::string_view code,
        size_t line,
        size_t & column,
        BetweenLinesData & commented_code_data
    ) noexcept
    {
        char const c = code[column];

        size_t const start = column;
        // true - comment like: // ...
        // false - comment like: /* ... */
        bool is_first_comment_type;

        if (commented_code_data.is_active)
        {
            is_first_comment_type = commented_code_data.is_first_comment_type;
        }
        if (!commented_code_data.is_active)
        {
            ++column;
            if (column >= code.size())
            {
                create_new_token_error(
                    token_errors,
                    "Error: undefined symbol on end of line",
                    { c },
                    line,
                    column
                );
                return;
            }
            char const next_char = code[column];
            ++column;

            if (next_char == '/')
            {
                is_first_comment_type = true;
            }
            else if (next_char == '*')
            {
                is_first_comment_type = false;
            }

            create_new_token_error(
                token_errors,
                "Error: undefined symbol or unstarting comment",
                { c, next_char },
                line,
                column
            );
            return;
        }

        bool is_previous_spesial_symbol = false;
        bool is_previous_star_symbol = false;

        while (column < code.size() && !(!is_first_comment_type && is_previous_star_symbol && code[column] == '/'))
        {
            if (is_previous_spesial_symbol)
            {
                is_previous_spesial_symbol = false;
            }
            else if (!is_previous_spesial_symbol && code[column] == '\\')
            {
                is_previous_spesial_symbol = true;
            }

            if (is_previous_star_symbol)
            {
                is_previous_star_symbol = false;
            }
            else if (!is_previous_star_symbol && code[column] == '*')
            {
                is_previous_star_symbol = true;
            }

            ++column;
        }

        if (column >= code.size() && !is_previous_spesial_symbol && is_first_comment_type)
        {
            std::string_view const word = code.substr(start, column - start);
            if (commented_code_data.is_active)
            {
                commented_code_data.data += std::string{ word };
                create_new_token(
                    symbol_table,
                    tokens,
                    commented_code_data.line,
                    commented_code_data.column,
                    TokenType::SingleLineComment,
                    commented_code_data.data
                );
                commented_code_data.is_active = false;
                return;
            }
            create_new_token(symbol_table, tokens, line, start, TokenType::SingleLineComment, word);
            return;
        }
        if (column >= code.size() && is_previous_spesial_symbol && is_first_comment_type)
        {
            if (commented_code_data.is_active)
            {
                commented_code_data.data += std::string{ code.substr(start, column - start - 1) };
                return;
            }
            commented_code_data.data = std::string{ code.substr(start, column - start - 1) };
            commented_code_data.line = line;
            commented_code_data.column = start;
            commented_code_data.is_first_comment_type = is_first_comment_type;
            commented_code_data.is_active = true;
            return;
        }
        if (column >= code.size() && !is_first_comment_type)
        {
            if (commented_code_data.is_active)
            {
                commented_code_data.data += std::string{ code.substr(start, column - start - 1) };
                return;
            }
            commented_code_data.data = std::string{ code.substr(start, column - start - 1) };
            commented_code_data.line = line;
            commented_code_data.column = start;
            commented_code_data.is_first_comment_type = is_first_comment_type;
            commented_code_data.is_active = true;
            return;
        }
        if (column < code.size())
        {
            ++column;

            std::string_view const word = code.substr(start, column - start);
            if (commented_code_data.is_active)
            {
                commented_code_data.data += std::string{ word };
                create_new_token(
                    symbol_table,
                    tokens,
                    commented_code_data.line,
                    commented_code_data.column,
                    TokenType::MultyLineComment,
                    commented_code_data.data
                );
                commented_code_data.is_active = false;
                return;
            }
            create_new_token(symbol_table, tokens, line, start, TokenType::MultyLineComment, word);
            return;
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
        if (column >= code.size())
        {
            if (node.type == TokenType::Invalid)
            {
                create_new_token_error(
                    token_errors,
                    "Error: invalid operator",
                    std::string{ code.substr(start, column - start) },
                    line,
                    column
                );
            }
            else
            {
                create_new_token(symbol_table, tokens, line, start, node.type);
            }
            return;
        }

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
                handle_operator_by_fa(symbol_table, tokens, token_errors, code, line, column, start, node.children[i]);
                return;
            }
        }
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


    void handle_word(
        symbol_table_t & symbol_table,
        tokens_t & tokens,
        token_errors_t & token_errors,
        std::string_view code,
        size_t line,
        size_t & column
    ) noexcept
    {
        bool has_number = false;

        size_t const start = column;
        ++column;
        while (column < code.size() && is_valid_word_part(code[column]))
        {
            if (!has_number && is_digit(code[column]))
            {
                has_number = true;
            }
            ++column;
        }

        std::string_view const word = code.substr(start, column - start);

        if (!has_number)
        {
            std::pair<TokenType, bool> const try_keywords = try_get_keywords(word);
            if (try_keywords.second)
            {
                create_new_token(symbol_table, tokens, line, start, try_keywords.first);
                return;
            }
        }

        create_new_token(symbol_table, tokens, line, start, TokenType::Id, word);
    }

    void handle_punctuation_marks(
        symbol_table_t & symbol_table,
        tokens_t & tokens,
        token_errors_t & token_errors,
        std::string_view code,
        size_t line,
        size_t & column
    ) noexcept
    {
        char const c = code[column];
        ++column;

        for (
            size_t i = static_cast<size_t>(TokenType::PunctuationMarksBegin) + 1;
            i < static_cast<size_t>(TokenType::PunctuationMarksEnd);
            ++i
            )
        {
            if (c == Token_to_string[i][0])
            {
                create_new_token(symbol_table, tokens, line, column, static_cast<TokenType>(i));
                return;
            }
        }
    }

    bool next_token(
        symbol_table_t & symbol_table,
        tokens_t & tokens,
        token_errors_t & token_errors,
        std::string_view code,
        size_t line,
        size_t & column,
        BetweenLinesData & commented_code_data,
        BetweenLinesData & string_constant_data,
        bool & is_now_preprocessor_directives
    ) noexcept
    {
        if (string_constant_data.is_active)
        {
            handle_string_constant(symbol_table, tokens, token_errors, code, line, column, string_constant_data);
            return !string_constant_data.is_active;
        }
        if (commented_code_data.is_active)
        {
            handle_comments(symbol_table, tokens, token_errors, code, line, column, commented_code_data);
            return !commented_code_data.is_active;
        }

        while (column < code.size() && is_space(code[column]))
        {
            ++column;
        }

        if (column == code.size())
        {
            if (is_now_preprocessor_directives && (code.empty() || code.back() != '\\'))
            {
                create_new_token(symbol_table, tokens, line, code.size(), TokenType::PrepDirEnd);
                is_now_preprocessor_directives = false;
            }
            return false;
        }

        char const c = code[column];


        if (is_valid_number_begin(c))
        {
            handle_digit(symbol_table, tokens, token_errors, code, line, column);
            return true;
        }
        if (c == '\'')
        {
            handle_literals_constant(symbol_table, tokens, token_errors, code, line, column);
            return true;
        }
        if (c == '\"')
        {
            handle_string_constant(symbol_table, tokens, token_errors, code, line, column, string_constant_data);
            return !string_constant_data.is_active;
        }
        if (c == '#')
        {
            handle_preprocessor_directives(symbol_table, tokens, token_errors, code, line, column, is_now_preprocessor_directives);
            return true;
        }
        if (c == '/')
        {
            handle_comments(symbol_table, tokens, token_errors, code, line, column, commented_code_data);
            return !commented_code_data.is_active;
        }
        if (is_valid_word_begin(c))
        {
            handle_word(symbol_table, tokens, token_errors, code, line, column);
            return true;
        }
        if (is_operator(c))
        {
            handle_operator_by_fa(symbol_table, tokens, token_errors, code, line, column);
            return true;
        }
        if (is_punctuation_marks(c))
        {
            handle_punctuation_marks(symbol_table, tokens, token_errors, code, line, column);
            return true;
        }

        if (is_now_preprocessor_directives && column + 1 == code.size() && c == '\\')
        {
            ++column;
            return true;
        }

        create_new_token_error(
            token_errors,
            "Error: \"tokem\" could not be recognized",
            { c },
            line,
            column
        );
        ++column;
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

        BetweenLinesData commented_code_data{};
        BetweenLinesData string_constant_data{};
        bool is_now_preprocessor_directives = false;

        std::string code;

        size_t line = 0;
        while (std::getline(file_input, code))
        {
            size_t column = 0;
            while (next_token(
                symbol_table,
                tokens,
                token_errors,
                code,
                line,
                column,
                commented_code_data,
                string_constant_data,
                is_now_preprocessor_directives
            ))
            {

            }
            ++line;
        }

        if (commented_code_data.is_active)
        {
            create_new_token_error(
                token_errors,
                "Error, unfinished comment",
                commented_code_data.data,
                commented_code_data.line,
                commented_code_data.column
            );
        }
        if (string_constant_data.is_active)
        {
            create_new_token_error(
                token_errors,
                "Error, unfinished string constant",
                string_constant_data.data,
                string_constant_data.line,
                string_constant_data.column
            );
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
                os << std::setw(50) << token_errors[i].message << ' ';
                os << "Symbol: " << std::setw(0) << '|' << token_errors[i].symbol << '|';
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
            if (is_symbol_type(tokens[i].type))
            {
                os << "Symbol id: " << std::setw(4) << std::to_string(tokens[i].index_in_symbol_table) << ' ';
                os << "Symbol: " << std::setw(0) << '|' << symbol_table[tokens[i].index_in_symbol_table] << '|';
            }
            else
            {
                os << "Symbol id: " << std::setw(4) << "" << ' ';
                os << "Symbol: " << std::setw(0);
            }

            os << '\n';
        }
    }
}