#pragma once

#include <BootstrapForge/Architecture/Grammar/LiteralKind.hpp>
#include <BootstrapForge/Architecture/Grammar/Operator.hpp>
#include <BootstrapForge/Architecture/Grammar/TokenKind.hpp>

#include <BootstrapForge/Architecture/Parsing/SourceSpan.hpp>

#include <compare>
#include <string>
#include <utility>

namespace BootstrapForge::Architecture::Parsing
{
    class Token final
    {
    public:

        Token() = default;

        Token(
            Grammar::TokenKind kind,
            std::string lexeme,
            SourceSpan span,
            Grammar::LiteralKind literalKind =
            Grammar::LiteralKind::None,
            Grammar::Operator op =
            Grammar::Operator{
                "",
                Grammar::TokenKind::Unknown })
                :
            m_kind(kind),
            m_literalKind(literalKind),
            m_operator(op),
            m_lexeme(std::move(lexeme)),
            m_span(span)
        {
        }

        [[nodiscard]]
        constexpr Grammar::TokenKind Kind() const noexcept
        {
            return m_kind;
        }

        [[nodiscard]]
        constexpr Grammar::LiteralKind LiteralKind() const noexcept
        {
            return m_literalKind;
        }

        [[nodiscard]]
        constexpr const Grammar::Operator& Operator() const noexcept
        {
            return m_operator;
        }

        [[nodiscard]]
        const std::string& Lexeme() const noexcept
        {
            return m_lexeme;
        }

        [[nodiscard]]
        constexpr const SourceSpan& Span() const noexcept
        {
            return m_span;
        }

        [[nodiscard]]
        constexpr bool Is(
            Grammar::TokenKind kind) const noexcept
        {
            return m_kind == kind;
        }

        [[nodiscard]]
        constexpr bool IsLiteral() const noexcept
        {
            return m_literalKind !=
                Grammar::LiteralKind::None;
        }

        [[nodiscard]]
        constexpr bool IsOperator() const noexcept
        {
            return m_operator.Token() !=
                Grammar::TokenKind::Unknown;
        }

        auto operator<=>(const Token&) const noexcept = default;

    private:

        Grammar::TokenKind m_kind =
            Grammar::TokenKind::Unknown;

        Grammar::LiteralKind m_literalKind =
            Grammar::LiteralKind::None;

        Grammar::Operator m_operator{
            "",
            Grammar::TokenKind::Unknown
        };

        std::string m_lexeme;

        SourceSpan m_span;
    };

} // namespace BootstrapForge::Architecture::Parsing