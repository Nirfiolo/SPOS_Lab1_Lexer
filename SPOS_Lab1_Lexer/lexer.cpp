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
        case lexer::TokenType::SharpInclude:
        case lexer::TokenType::SharpDefine:
        case lexer::TokenType::SharpError:
        case lexer::TokenType::SharpImport:
        case lexer::TokenType::SharpLine:
        case lexer::TokenType::SharpPragma:
        case lexer::TokenType::SharpUsing:
        case lexer::TokenType::SharpIf:
        case lexer::TokenType::SharpIfdef:
        case lexer::TokenType::SharpIfndef:
        case lexer::TokenType::SharpElif:
        case lexer::TokenType::SharpElse:
        case lexer::TokenType::SharpUndef:
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
            return true;
        default:
            return false;
        }
    }

    bool is_end_of_multi_line_preprocessor_directives(TokenType type) noexcept
    {
        return is_multi_line_preprocessor_directives(type) || type == TokenType::SharpEndif;
    }

    bool is_single_word_preprocessor_directives(TokenType type) noexcept
    {
        return type == lexer::TokenType::SharpEndif;
    }


    struct FAState
    {
        char c{ '\0' };
        TokenType type{ TokenType::Invalid };

        std::vector<FAState> children{};
    };

    static FAState fa_start{};

    void generate_fa_state(
        std::vector<std::pair<std::string, TokenType>> const & string_to_type,
        FAState & state,
        size_t offset,
        size_t & position
    ) noexcept
    {
        do
        {
            size_t const last_poistion = position;
            if (string_to_type[position].first.size() > offset)
            {
                FAState new_state{};
                generate_fa_state(string_to_type, new_state, offset + 1, position);
                state.children.emplace_back(std::move(new_state));
            }
            else
            {
                state.c = string_to_type[position].first[offset - 1];
                state.type = string_to_type[position].second;
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
        generate_fa_state(string_to_type, fa_start, 0, position);
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

    struct CommonData
    {
        symbol_table_t symbol_table{};
        tokens_t tokens{};
        token_errors_t token_errors{};
        std::string_view code{};
        size_t line{ 0 };
        size_t column{ 0 };
    };

    struct BetweenLinesData
    {
        std::string data{ "" };
        size_t line;
        size_t column;
        bool is_active{ false };
        TokenType type{ TokenType::Invalid };
    };

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

    void create_new_token(
        symbol_table_t & symbol_table,
        tokens_t & tokens,
        BetweenLinesData const & between_lines_data
    ) noexcept
    {
        create_new_token(symbol_table,
            tokens,
            between_lines_data.line,
            between_lines_data.column,
            between_lines_data.type,
            between_lines_data.data
        );
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
        std::string const & message,
        BetweenLinesData const & between_lines_data
    ) noexcept
    {
        create_new_token_error(
            token_errors,
            message,
            between_lines_data.data,
            between_lines_data.line,
            between_lines_data.column
        );
    }

    void handle_operator_by_fa(CommonData & data) noexcept;

    void handle_digit(CommonData & data) noexcept
    {
        char const c = data.code[data.column];

        size_t const start = data.column;
        ++data.column;

        bool has_dot = (c == '.');
        size_t dot_position = (has_dot ? data.column : std::numeric_limits<size_t>::max());
        bool is_decimal = true;
        bool is_hex = false;
        bool is_binary = false;
        bool const is_first_zero = (c == '0');

        size_t last_number_separator_index = start;

        if (data.column >= data.code.size())
        {
            if (has_dot)
            {
                --data.column;
                handle_operator_by_fa(data);
                return;
            }
            create_new_token(data.symbol_table, data.tokens, data.line, start, TokenType::IntNumber, data.code.substr(start, 1));
            return;
        }

        char const next_char = data.code[data.column];
        if (has_dot && !is_digit(next_char))
        {
            --data.column;
            handle_operator_by_fa(data);
            return;
        }
        if (!is_first_zero && !has_dot && !is_valid_number_begin(next_char))
        {
            create_new_token(data.symbol_table, data.tokens, data.line, start, TokenType::IntNumber, data.code.substr(start, 1));
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
            create_new_token(data.symbol_table, data.tokens, data.line, start, TokenType::IntNumber, data.code.substr(start, 1));
            return;
        }

        if (next_char == '\'')
        {
            last_number_separator_index = data.column;
        }

        if (next_char == '.')
        {
            has_dot = true;
            dot_position = data.column;
        }

        ++data.column;

        while (data.column < data.code.size() &&
            ((is_decimal && is_valid_number_part(data.code[data.column])) ||
                (is_hex && is_valid_hex_number_part(data.code[data.column])) ||
                (is_binary && is_valid_binary_number_part(data.code[data.column]))))
        {
            if (has_dot && data.code[data.column] == '.')
            {
                ++data.column;
                create_new_token_error(
                    data.token_errors,
                    "Error: double dot in number value",
                    std::string{ data.code.substr(start, data.column - start) },
                    data.line,
                    start
                );
                return;
            }
            if (!has_dot && data.code[data.column] == '.')
            {
                if (data.column - last_number_separator_index == 1)
                {
                    ++data.column;
                    create_new_token_error(
                        data.token_errors,
                        "Error: number separator and dot too close",
                        std::string{ data.code.substr(start, data.column - start) },
                        data.line,
                        start
                    );
                    return;
                }
                has_dot = true;
                dot_position = data.column;
            }
            if (data.code[data.column] == '\'')
            {
                if (data.column - last_number_separator_index == 1)
                {
                    ++data.column;
                    create_new_token_error(
                        data.token_errors,
                        "Error: number separators too close",
                        std::string{ data.code.substr(start, data.column - start) },
                        data.line,
                        start
                    );
                    return;
                }
                if (data.column - dot_position == 1)
                {
                    ++data.column;
                    create_new_token_error(
                        data.token_errors,
                        "Error: dot and number separator too close",
                        std::string{ data.code.substr(start, data.column - start) },
                        data.line,
                        start
                    );
                    return;
                }
                last_number_separator_index = data.column;
            }
            ++data.column;
        }

        if (data.column < data.code.size() && !is_valid_symbol_after_number(data.code[data.column]))
        {
            ++data.column;
            create_new_token_error(
                data.token_errors,
                "Error: invalid symbol after number",
                std::string{ data.code.substr(start, data.column - start) },
                data.line,
                start
            );
            return;
        }
        if (!((is_decimal && is_digit(data.code[data.column - 1])) ||
            (is_hex && is_hex_number(data.code[data.column - 1])) ||
            (is_binary && is_binary_number(data.code[data.column - 1]))))
        {
            create_new_token_error(
                data.token_errors,
                "Error: invalid number end",
                std::string{ data.code.substr(start, data.column - start) },
                data.line,
                start
            );
            return;
        }

        std::string_view const number = data.code.substr(start, data.column - start);
        if (has_dot)
        {
            create_new_token(data.symbol_table, data.tokens, data.line, start, TokenType::FloatNumber, number);
        }
        if (!has_dot)
        {
            create_new_token(data.symbol_table, data.tokens, data.line, start, TokenType::IntNumber, number);
        }
    }

    void handle_literals_constant(CommonData & data) noexcept
    {
        char const c = data.code[data.column];

        size_t const start = data.column;
        ++data.column;
        if (data.column >= data.code.size())
        {
            create_new_token_error(
                data.token_errors,
                "Error: unfinished symbol: symbol on end of line",
                std::string{ c },
                data.line,
                data.column
            );
            return;
        }
        char next_char = data.code[data.column];

        if (next_char == '\'')
        {
            create_new_token_error(
                data.token_errors,
                "Error: empty character constant",
                std::string{ c, next_char },
                data.line,
                data.column
            );
            return;
        }
        
        bool const is_need_additional_char = (next_char == '\\');
        char additional_char;
        if (is_need_additional_char)
        {
            ++data.column;
            additional_char = next_char;

            if (data.column >= data.code.size())
            {
                create_new_token_error(
                    data.token_errors,
                    "Error: unfinished symbol: symbols on end of line",
                    std::string{ c, additional_char },
                    data.line,
                    data.column
                );
                return;
            }
            next_char = data.code[data.column];
        }

        ++data.column;
        if (data.column >= data.code.size())
        {
            if (is_need_additional_char)
            {
                create_new_token_error(
                    data.token_errors,
                    "Error: unfinished symbol: symbols on end of line",
                    std::string{ c, additional_char, next_char },
                    data.line,
                    data.column
                );
                return;
            }
            create_new_token_error(
                data.token_errors,
                "Error: unfinished symbol: symbols on end of line",
                std::string{ c, next_char },
                data.line,
                data.column
            );
            return;
        }
        char const last_char = data.code[data.column];

        if (last_char != '\'')
        {
            if (is_need_additional_char)
            {
                create_new_token_error(
                    data.token_errors,
                    "Error: too many characters in symbol constant",
                    std::string{ c, additional_char, next_char, last_char },
                    data.line,
                    data.column
                );
                return;
            }
            create_new_token_error(
                data.token_errors,
                "Error: too many characters in symbol constant",
                std::string{ c, next_char, last_char },
                data.line,
                data.column
            );
            return;
        }
        ++data.column;

        std::string_view const word = data.code.substr(start, data.column - start);

        create_new_token(data.symbol_table, data.tokens, data.line, start, TokenType::Character, word);
    }

    void handle_string_constant(CommonData & data, BetweenLinesData & string_constant_data) noexcept
    {
        size_t const start = data.column;
        if (!string_constant_data.is_active)
        {
            ++data.column;
        }

        bool is_previous_spesial_symbol = false;

        while (data.column < data.code.size() && !(!is_previous_spesial_symbol && data.code[data.column] == '\"'))
        {
            if (is_previous_spesial_symbol)
            {
                is_previous_spesial_symbol = false;
            }
            else if (!is_previous_spesial_symbol && data.code[data.column] == '\\')
            {
                is_previous_spesial_symbol = true;
            }
            ++data.column;
        }

        if (data.column >= data.code.size() && is_previous_spesial_symbol)
        {
            std::string text = std::string{ data.code.substr(start, data.column - start - 1) };
            if (string_constant_data.is_active)
            {
                string_constant_data.data += std::move(text);
                return;
            }
            string_constant_data.data = std::move(text);
            string_constant_data.line = data.line;
            string_constant_data.column = start;
            string_constant_data.is_active = true;
            return;
        }
        else if (data.column >= data.code.size())
        {
            if (!string_constant_data.is_active)
            {
                string_constant_data.data = "";
                string_constant_data.line = data.line;
                string_constant_data.column = data.column;
            }
            string_constant_data.data += std::string{ data.code.substr(start, data.column - start) };
            string_constant_data.is_active = false;

            create_new_token_error(
                data.token_errors,
                "Error: unfinished string constant",
                string_constant_data
            );
            return;
        }

        ++data.column;
        std::string_view const word = data.code.substr(start, data.column - start);

        if (string_constant_data.is_active)
        {
            string_constant_data.data += std::string{ word };
            string_constant_data.type = TokenType::String;
            create_new_token(
                data.symbol_table,
                data.tokens,
                string_constant_data
            );
            string_constant_data.is_active = false;
            return;
        }

        create_new_token(data.symbol_table, data.tokens, data.line, start, TokenType::String, word);
    }

    std::pair<TokenType, bool> try_handle_preprocessor_word(CommonData & data) noexcept
    {
        size_t const start = data.column;
        ++data.column;
        while (data.column < data.code.size() && is_lower(data.code[data.column]))
        {
            ++data.column;
        }

        std::string_view const word = data.code.substr(start, data.column - start);

        return try_get_preprocessor_directives(word);
    }

    void handle_preprocessor_directives(CommonData & data, BetweenLinesData & preprocessor_directives_data) noexcept
    {
        size_t const start = data.column;

        TokenType type{ TokenType::Invalid };
        bool is_emptpy_line = false;
        bool is_first_line = true;

        if (preprocessor_directives_data.is_active)
        {
            type = preprocessor_directives_data.type;
            is_emptpy_line = true;
            is_first_line = false;
        }
        else
        {
            std::pair<TokenType, bool> const preprocessor_directives = try_handle_preprocessor_word(data);

            if (!preprocessor_directives.second)
            {
                std::string_view const word = data.code.substr(start, data.column - start);
                create_new_token_error(
                    data.token_errors,
                    "Error: undefined preprocessor directives",
                    std::string{ word },
                    data.line,
                    data.column
                );
                return;
            }

            type = preprocessor_directives.first;
            preprocessor_directives_data.type = type;
            if (is_single_word_preprocessor_directives(type))
            {
                create_new_token(data.symbol_table, data.tokens, data.line, start, type);
                return;
            }
        }

        if (is_multi_line_preprocessor_directives(type) && is_emptpy_line)
        {
            while (is_emptpy_line && data.column < data.code.size() && is_space(data.code[data.column]))
            {
                ++data.column;
            }

            if (data.column < data.code.size() && data.code[data.column] == '#')
            {
                size_t const current_column = data.column;

                std::pair<TokenType, bool> const preprocessor_directives = try_handle_preprocessor_word(data);
                if (is_end_of_multi_line_preprocessor_directives(preprocessor_directives.first))
                {
                    create_new_token(
                        data.symbol_table,
                        data.tokens,
                        preprocessor_directives_data
                    );
                    preprocessor_directives_data.is_active = false;

                    data.column = current_column;
                    return;
                }
            }
            is_emptpy_line = false;
        }

        while (data.column < data.code.size())
        {
            ++data.column;
        }

        if (data.column >= data.code.size() && !is_multi_line_preprocessor_directives(type))
        {
            if (data.code.back() == '\\')
            {
                std::string text = std::string{ data.code.substr(start, data.column - start - 1) };
                if (preprocessor_directives_data.is_active)
                {
                    preprocessor_directives_data.data += std::move(text);
                    return;
                }
                preprocessor_directives_data.data = std::move(text);
                preprocessor_directives_data.line = data.line;
                preprocessor_directives_data.column = start;
                preprocessor_directives_data.is_active = true;
                return;
            }

            std::string_view const text = data.code.substr(start, data.column - start);
            if (preprocessor_directives_data.is_active)
            {
                preprocessor_directives_data.data += std::string{ text };
                create_new_token(
                    data.symbol_table,
                    data.tokens,
                    preprocessor_directives_data
                );
                preprocessor_directives_data.is_active = false;
                return;
            }
            create_new_token(data.symbol_table, data.tokens, data.line, start, type, text);
            return;
        }

        if (data.column >= data.code.size() && is_multi_line_preprocessor_directives(type))
        {
            std::string text = std::string{ data.code.substr(start, data.column - start) };
            if (preprocessor_directives_data.is_active)
            {
                preprocessor_directives_data.data += std::move(text);
                return;
            }
            preprocessor_directives_data.data = std::move(text);
            preprocessor_directives_data.line = data.line;
            preprocessor_directives_data.column = start;
            preprocessor_directives_data.is_active = true;

            if (is_first_line)
            {
                preprocessor_directives_data.data += '\n';
            }

            return;
        }
    }

    void handle_comments(CommonData & data, BetweenLinesData & commented_code_data) noexcept
    {
        char const c = data.code[data.column];

        size_t const start = data.column;
        // true - comment like: // ...
        // false - comment like: /* ... */
        bool is_first_type;
        TokenType type;

        if (commented_code_data.is_active)
        {
            is_first_type = (commented_code_data.type == TokenType::SingleLineComment);
            type = commented_code_data.type;
        }
        if (!commented_code_data.is_active)
        {
            ++data.column;
            if (data.column >= data.code.size())
            {
                --data.column;
                handle_operator_by_fa(data);
                return;
            }
            char const next_char = data.code[data.column];
            ++data.column;

            if (next_char == '/')
            {
                is_first_type = true;
                type = TokenType::SingleLineComment;
                commented_code_data.type = type;
            }
            else if (next_char == '*')
            {
                is_first_type = false;
                type = TokenType::MultyLineComment;
                commented_code_data.type = type;
            }
            else
            {
                --data.column;
                handle_operator_by_fa(data);
                return;
            }
        }

        bool is_previous_spesial_symbol = false;
        bool is_previous_star_symbol = false;

        while (data.column < data.code.size() && !(!is_first_type && is_previous_star_symbol && data.code[data.column] == '/'))
        {
            if (is_previous_spesial_symbol)
            {
                is_previous_spesial_symbol = false;
            }
            else if (!is_previous_spesial_symbol && data.code[data.column] == '\\')
            {
                is_previous_spesial_symbol = true;
            }

            if (is_previous_star_symbol)
            {
                is_previous_star_symbol = false;
            }
            else if (!is_previous_star_symbol && data.code[data.column] == '*')
            {
                is_previous_star_symbol = true;
            }

            ++data.column;
        }

        if (data.column >= data.code.size() && !is_previous_spesial_symbol && is_first_type)
        {
            std::string_view const word = data.code.substr(start, data.column - start);
            if (commented_code_data.is_active)
            {
                commented_code_data.data += std::string{ word };
                create_new_token(
                    data.symbol_table,
                    data.tokens,
                    commented_code_data
                );
                commented_code_data.is_active = false;
                return;
            }
            create_new_token(data.symbol_table, data.tokens, data.line, start, TokenType::SingleLineComment, word);
            return;
        }
        if (data.column >= data.code.size() && ((is_previous_spesial_symbol && is_first_type) || !is_first_type))
        {
            std::string text = std::string{ data.code.substr(start, data.column - start - 1) };
            if (commented_code_data.is_active)
            {
                commented_code_data.data += std::move(text);
                return;
            }
            commented_code_data.data = std::move(text);
            commented_code_data.line = data.line;
            commented_code_data.column = start;
            commented_code_data.is_active = true;
            return;
        }
        if (data.column < data.code.size())
        {
            ++data.column;

            std::string_view const word = data.code.substr(start, data.column - start);
            if (commented_code_data.is_active)
            {
                commented_code_data.data += std::string{ word };
                create_new_token(
                    data.symbol_table,
                    data.tokens,
                    commented_code_data
                );
                commented_code_data.is_active = false;
                return;
            }
            create_new_token(data.symbol_table, data.tokens, data.line, start, type, word);
            return;
        }
    }

    void handle_operator_by_fa(CommonData & data, size_t start, FAState const & state) noexcept
    {
        if (data.column >= data.code.size())
        {
            if (state.type == TokenType::Invalid)
            {
                create_new_token_error(
                    data.token_errors,
                    "Error: invalid operator",
                    std::string{ data.code.substr(start, data.column - start) },
                    data.line,
                    data.column
                );
            }
            else
            {
                create_new_token(data.symbol_table, data.tokens, data.line, start, state.type);
            }
            return;
        }

        char const current_char = data.code[data.column];
        bool const is_current_char_operator = is_operator(current_char);

        if (!is_current_char_operator)
        {
            create_new_token(data.symbol_table, data.tokens, data.line, start, state.type);
            return;
        }

        ++data.column;
        for (size_t i = 0; i < state.children.size(); ++i)
        {
            if (state.children[i].c == current_char)
            {
                handle_operator_by_fa(data, start, state.children[i]);
                return;
            }
        }

        if (state.type == TokenType::Invalid)
        {
            create_new_token_error(
                data.token_errors,
                "Error: invalid operator",
                std::string{ data.code.substr(start, data.column - start) },
                data.line,
                data.column
            );
        }
        else
        {
            create_new_token(data.symbol_table, data.tokens, data.line, start, state.type);
        }
        --data.column;
    }

    void handle_operator_by_fa(CommonData & data) noexcept
    {
        handle_operator_by_fa(data, data.column, fa_start);
    }

    void handle_word(CommonData & data) noexcept
    {
        bool has_number = false;

        size_t const start = data.column;
        ++data.column;
        while (data.column < data.code.size() && is_valid_word_part(data.code[data.column]))
        {
            if (!has_number && is_digit(data.code[data.column]))
            {
                has_number = true;
            }
            ++data.column;
        }

        std::string_view const word = data.code.substr(start, data.column - start);

        if (!has_number)
        {
            std::pair<TokenType, bool> const try_keywords = try_get_keywords(word);
            if (try_keywords.second)
            {
                create_new_token(data.symbol_table, data.tokens, data.line, start, try_keywords.first);
                return;
            }
        }

        create_new_token(data.symbol_table, data.tokens, data.line, start, TokenType::Id, word);
    }

    void handle_punctuation_marks(CommonData & data) noexcept
    {
        char const c = data.code[data.column];
        ++data.column;

        for (
            size_t i = static_cast<size_t>(TokenType::PunctuationMarksBegin) + 1;
            i < static_cast<size_t>(TokenType::PunctuationMarksEnd);
            ++i
            )
        {
            if (c == Token_to_string[i][0])
            {
                create_new_token(data.symbol_table, data.tokens, data.line, data.column, static_cast<TokenType>(i));
                return;
            }
        }
    }

    bool next_token(
        CommonData & data,
        BetweenLinesData & commented_code_data,
        BetweenLinesData & string_constant_data,
        BetweenLinesData & preprocessor_directives_data
    ) noexcept
    {
        if (string_constant_data.is_active)
        {
            handle_string_constant(data, string_constant_data);
            return !string_constant_data.is_active;
        }
        if (commented_code_data.is_active)
        {
            handle_comments(data, commented_code_data);
            return !commented_code_data.is_active;
        }
        if (preprocessor_directives_data.is_active)
        {
            handle_preprocessor_directives(data, preprocessor_directives_data);
            return !preprocessor_directives_data.is_active;
        }

        while (data.column < data.code.size() && is_space(data.code[data.column]))
        {
            ++data.column;
        }

        if (data.column >= data.code.size())
        {
            return false;
        }

        char const c = data.code[data.column];

        // TODO: fixed copy-paste

        // TODO FA into handles

        if (is_valid_number_begin(c))
        {
            handle_digit(data);
            return true;
        }
        if (c == '\'')
        {
            handle_literals_constant(data);
            return true;
        }
        if (c == '\"')
        {
            handle_string_constant(data, string_constant_data);
            return !string_constant_data.is_active;
        }
        if (c == '#')
        {
            handle_preprocessor_directives(data, preprocessor_directives_data);
            return !preprocessor_directives_data.is_active;
        }
        if (c == '/')
        {
            // TODO: dilive
            handle_comments(data, commented_code_data);
            return !commented_code_data.is_active;
        }
        if (is_valid_word_begin(c))
        {
            handle_word(data);
            return true;
        }
        if (is_operator(c))
        {
            handle_operator_by_fa(data);
            return true;
        }
        if (is_punctuation_marks(c))
        {
            handle_punctuation_marks(data);
            return true;
        }

        // TODO: more info
        create_new_token_error(
            data.token_errors,
            // TODO: fixed typo
            "Error: \"tokem\" could not be recognized",
            { c },
            data.line,
            data.column
        );
        ++data.column;
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

        if (fa_start.children.empty())
        {
            generate_fa();
        }

        CommonData data{};

        BetweenLinesData commented_code_data{};
        BetweenLinesData string_constant_data{};
        BetweenLinesData preprocessor_directives_data{};

        std::string code;

        while (std::getline(file_input, code))
        {
            data.code = code;
            data.column = 0;
            while (next_token(
                data,
                commented_code_data,
                string_constant_data,
                preprocessor_directives_data
            ))
            {

            }
            ++data.line;
        }

        if (commented_code_data.is_active)
        {
            create_new_token_error(
                data.token_errors,
                "Error, unfinished comment",
                commented_code_data
            );
        }
        if (string_constant_data.is_active)
        {
            create_new_token_error(
                data.token_errors,
                "Error, unfinished string constant",
                string_constant_data
            );
        }
        if (preprocessor_directives_data.is_active)
        {
            create_new_token_error(
                data.token_errors,
                "Error, unfinished preprocessor directives",
                preprocessor_directives_data
            );
        }

        return { data.symbol_table, { data.tokens, data.token_errors } };
    }

    // TODO: fixed token output
    // firstly only data.tokens
    // secondly all other
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