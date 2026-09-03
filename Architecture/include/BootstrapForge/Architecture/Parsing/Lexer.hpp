#pragma once

#include <BootstrapForge/Architecture/Grammar/Grammar.hpp>

#include <BootstrapForge/Architecture/Parsing/SourceReader.hpp>
#include <BootstrapForge/Architecture/Parsing/Token.hpp>

#include <cstddef>
#include <string_view>

namespace BootstrapForge::Architecture::Parsing
{
    class Lexer final
    {
    public:

        Lexer(
            const Grammar::Grammar& grammar,
            std::string_view source) noexcept;

        [[nodiscard]]
        Token NextToken();

    private:

        void SkipWhitespace();

        void SkipLineComment();

        [[nodiscard]]
        Token ReadIdentifier();

        [[nodiscard]]
        Token ReadNumber();

        [[nodiscard]]
        Token ReadString();

        [[nodiscard]]
        Token ReadOperator();

        [[nodiscard]]
        Token ReadUnknown();

        [[nodiscard]]
        Token MakeToken(
            Grammar::TokenKind kind,
            SourceLocation begin,
            Grammar::LiteralKind literalKind =
            Grammar::LiteralKind::None,
            Grammar::Operator op =
            Grammar::Operator{
                "",
                Grammar::TokenKind::Unknown });

        [[nodiscard]]
        static bool IsIdentifierStart(
            char character) noexcept;

        [[nodiscard]]
        static bool IsIdentifierPart(
            char character) noexcept;

        [[nodiscard]]
        static bool IsDigit(
            char character) noexcept;

        [[nodiscard]]
        static bool IsWhitespace(
            char character) noexcept;

        const Grammar::Grammar& m_grammar;

        SourceReader m_reader;
    };

} // namespace BootstrapForge::Architecture::Parsing