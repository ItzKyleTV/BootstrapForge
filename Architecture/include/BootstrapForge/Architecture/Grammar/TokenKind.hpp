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
    RawStringLiteral,
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
        Minus,

        //
        // Keywords
        //

        KeywordArchitecture,

        KeywordGenerate,

        KeywordProject,
        KeywordModule,

        KeywordDirectory,
        KeywordFile,

        KeywordBranch,
        KeywordSubbranch,
        KeywordFolder,
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
