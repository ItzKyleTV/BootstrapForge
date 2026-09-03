#pragma once

#include <cstdint>

namespace BootstrapForge::Architecture::Grammar
{
    enum class TokenKind : std::uint16_t
    {
        //
        // Special
        //

        Unknown = 0,
        EndOfFile,

        //
        // Identifiers
        //

        Identifier,

        //
        // Literals
        //

        IntegerLiteral,
        FloatLiteral,
        StringLiteral,
        BooleanLiteral,
        PathLiteral,

        //
        // Delimiters
        //

        LeftParenthesis,
        RightParenthesis,

        LeftBrace,
        RightBrace,

        LeftBracket,
        RightBracket,

        Comma,
        Colon,
        DoubleColon,
        Semicolon,
        Dot,

        //
        // Operators
        //

        Equals,
        Arrow,
        ForwardSlash,
        BackSlash,

        //
        // Keywords
        //

        KeywordArchitecture,

        KeywordProject,
        KeywordModule,

        KeywordDirectory,
        KeywordFile,

        KeywordBranch,
        KeywordRelationship,

        KeywordBuild,
        KeywordGenerator,

        KeywordCreate,
        KeywordModify,
        KeywordRemove,
        KeywordRename,
        KeywordMove,
        KeywordCopy,

        KeywordInclude,

        //
        // Comments
        //

        Comment
    };

} // namespace BootstrapForge::Architecture::Grammar