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
        Character,
        String,

        // * preprocessor directives
        PreprocessorDirectivesBegin,

        SharpInclude,
        SharpDefine,
        SharpError,
        SharpImport,
        SharpLine,
        SharpPragma,
        SharpUsing,
        SharpIf,
        SharpIfdef,
        SharpIfndef,
        SharpEndif,
        SharpElif,
        SharpElse,
        SharpUndef,

        PreprocessorDirectivesEnd,

        // * comments
        SingleLineComment,
        MultyLineComment,

        // * keywords
        KeywordsBegin,

        // types
        Bool,
        Char,
        Short,
        Int,
        Long,
        Unsigned,
        Float,
        Double,
        Struct,
        Class,
        Enum,
        Auto,
        Void,

        // access modifier
        Public,
        Protected,
        Private,


        Do,
        While,
        For,
        If,
        Else,
        Continue,
        Break,
        Return,
        Default,

        Typeid,
        Using,

        True,
        False,

        Const,
        Volatile,
        Constexpr,
        Static,

        // exceptions
        Noexcept,
        Throw,

        // "Four Horsemen"
        Static_cast,
        Const_cast,
        Dynamic_cast,
        Reinterpret_cast,

        KeywordsEnd,

        // * identifiers
        Id,

        // * operators
        OperatorsBegin,

        Association,
        // arithmetic
        Add,
        Increment,
        Sub,
        Decrement,
        Multiply,
        Divide,
        Mod,
        AddAssociation,
        SubAssociation,
        MultiplyAssociation,
        DivideAssociation,
        ModAssociation,

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
        BinaryShiftToLeft,
        BinaryShiftToRight,
        BinaryAndAssociation,
        BinaryOrAssociation,
        BinaryNotAssociation,
        BinaryXorAssociation,
        BinaryShiftToLeftAssociation,
        BinaryShiftToRightAssociation,
        // conditional ( ? : )
        QUEST,
        Colon,
        // others
        Dot,
        MemberAccess,
        Scope,
        Dotted,

        OperatorsEnd,

        // * punctuation marks
        PunctuationMarksBegin,

        Comma,
        Semicolon,
        LeftParen,
        RightParen,
        LeftBrack,
        RightBrack,
        LeftBrace,
        RightBrace,

        PunctuationMarksEnd,

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
        "Character",
        "String",

        // * preprocessor directives
        "PreprocessorDirectivesBegin",

        "#include",
        "#define",
        "#error",
        "#import",
        "#line",
        "#pragma",
        "#using",
        "#if",
        "#ifdef",
        "#ifndef",
        "#endif",
        "#elif",
        "#else",
        "#undef",

        "PreprocessorDirectivesEnd",

        // * comments
        "// ...",
        "/* ... */",

        // * keywords
        "KeywordsBegin",

        // types
        "bool",
        "char",
        "short",
        "int",
        "long",
        "unsigned",
        "float",
        "double",
        "struct",
        "class",
        "enum",
        "auto",
        "void",

        // access modifier
        "public",
        "protected",
        "private",


        "do",
        "while",
        "for",
        "if",
        "else",
        "continue",
        "break",
        "return",
        "default",

        "typeid",
        "using",

        "true",
        "false",

        "const",
        "volatile",
        "constexpr",
        "static",

        // exceptions
        "noexcept",
        "throw",

        // "Four Horsemen"
        "static_cast",
        "const_cast",
        "dynamic_cast",
        "reinterpret_cast",

        "KeywordsEnd",

        // * identifiers
        "Id",

        // * operators
        "OperatorsBegin",

        "=",
        // arithmetic
        "+",
        "++",
        "-",
        "--",
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
        "<<",
        ">>",
        "&=",
        "|=",
        "~=",
        "^=",
        "<<=",
        ">>=",
        // conditional ( ? : )
        "?",
        ":",
        // others
        ".",
        "->",
        "::",
        "...",

        "OperatorsEnd",

        // * punctuation marks
        "PunctuationMarksBegin",

        ",",
        ";",
        "(",
        ")",
        "[",
        "]",
        "{",
        "}",

        "PunctuationMarksEnd",

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
        std::string symbol;
        size_t line;
        size_t column;
        size_t length;

        TokenError(std::string message, std::string symbol, size_t line, size_t column, size_t length) noexcept
            : message{ message },
            symbol{ symbol },
            line{ line },
            column{ column },
            length{ length }
        {

        }
    };


    using symbol_table_t = std::vector<std::string>;
    using tokens_t = std::vector<Token>;
    using token_errors_t = std::vector<TokenError>;

    using lexer_output_t = std::pair<symbol_table_t, std::pair<tokens_t, token_errors_t>>;

    lexer_output_t get_tokens(std::string const & file_path) noexcept(!IS_DEBUG);

    void output_lexer_data(std::ostream & os, lexer_output_t const & lexer_output) noexcept;
}