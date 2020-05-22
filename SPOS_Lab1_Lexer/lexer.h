#pragma once


#include <vector>
#include <string>


namespace lexer
{
    enum class TokenType : uint8_t
    {
        // * numeric constants
        NumericConstantsBegin,

        IntNumber,
        FloatNumber,

        NumericConstantsEnd,

        // * literals and symbolic constants

        // * preprocessor directives

        // * comments

        // * keywords
        KeywordsBegin,

        Int,
        Float,

        KeywordsEnd,

        // * identifiers
        Id,

        // * operators
        OperatorsBegin,

        Association,
        // arithmetic
        Add,
        Sub,
        Multiply,
        Divide,
        Mod,
        AddAssociation,
        SubAssociation,
        MultiplyAssociation,
        DivideAssociation,
        ModAssociation,
        // TODO: unar - +

        // logic
        And,
        Or,
        Not,
        Equal,
        Less,
        Great,
        LessEqual,
        GreatEqual,
        NotEqual,
        // bitwise
        BinaryAnd,
        BinaryOr,
        BinaryNot,
        BinaryXor,
        BinaryAndAssociation,
        BinaryOrAssociation,
        BinaryNotAssociation,
        BinaryXorAssociation,
        // conditional ( ? : )
        QUEST,
        Colon,
        // others
        LeftShift,
        RightShift,
        LeftShiftAssociation,
        RightShiftAssociation,

        OperatorsEnd,

        // * punctuation marks

        // * invalid
        Invalid,

        CountOf
    };

    constexpr char const * Token_to_string[static_cast<uint8_t>(TokenType::CountOf)] =
    {
        // * numeric constants
        "NumericConstantsBegin",

        "IntNumber",
        "FloatNumber",

        "NumericConstantsEnd",

        // * literals and symbolic constants

        // * preprocessor directives

        // * comments

        // * keywords
        "KeywordsBegin",

        "int",
        "float",

        "KeywordsEnd",

        // * identifiers
        "Id",

        // * operators
        "OperatorsBegin",

        "=",
        // arithmetic
        "+",
        "-",
        "*",
        "/",
        "%",
        "+=",
        "-=",
        "*=",
        "/=",
        "%=",
        // logic
        "&&",
        "||",
        "!",
        "==",
        "<",
        ">",
        "<=",
        ">=",
        "!=",
        // bitwise
        "&",
        "|",
        "~",
        "^",
        "&=",
        "|=",
        "~=",
        "^=",
        // conditional ( ? : )
        "?",
        ":",
        // others
        "<<",
        ">>",
        "<<=",
        ">>=",

        "OperatorsEnd",

        // * punctuation marks

        // * invalid
        "Invalid"
    };

    struct Token
    {
        size_t line;
        size_t column;
        size_t index_in_symbol_table;
        TokenType type;

        Token(
            size_t line,
            size_t column,
            TokenType type,
            size_t index_in_symbol_table = std::numeric_limits<size_t>::max()
        ) noexcept
            : line{ line },
            column{ column },
            type{ type },
            index_in_symbol_table{ index_in_symbol_table }
        {

        }
    };

    struct TokenError
    {
        std::string message;
        size_t line;
        size_t column;
    };


    using symbol_table_t = std::vector<std::string>;
    using tokens_t = std::vector<Token>;
    using token_errors_t = std::vector<TokenError>;

    using lexer_output_t = std::pair<symbol_table_t, std::pair<tokens_t, token_errors_t>>;

    lexer_output_t get_tokens(std::string const & file_path) noexcept(!IS_DEBUG);


    void output_lexer_data(std::ostream & os, lexer_output_t const & lexer_output) noexcept;
}