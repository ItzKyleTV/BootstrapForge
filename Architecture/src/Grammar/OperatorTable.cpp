#include <BootstrapForge/Architecture/Grammar/OperatorTable.hpp>

namespace BootstrapForge::Architecture::Grammar
{
    OperatorTable::OperatorTable()
    {
        m_operators =
        {
            { "{",  TokenKind::LeftBrace },
            { "}",  TokenKind::RightBrace },

            { "(",  TokenKind::LeftParenthesis },
            { ")",  TokenKind::RightParenthesis },

            { "[",  TokenKind::LeftBracket },
            { "]",  TokenKind::RightBracket },

            { ",",  TokenKind::Comma },
            { ":",  TokenKind::Colon },
            { "::", TokenKind::DoubleColon },
            { ";",  TokenKind::Semicolon },
            { ".",  TokenKind::Dot },

            { "=",  TokenKind::Equals },
            { "->", TokenKind::Arrow },

            { "/",  TokenKind::ForwardSlash },
            { "\\", TokenKind::BackSlash }
        };
    }

    std::optional<Operator> OperatorTable::Find(
        std::string_view text) const noexcept
    {
        for (const Operator& op : m_operators)
        {
            if (op.Text() == text)
            {
                return op;
            }
        }

        return std::nullopt;
    }

    const std::vector<Operator>&
        OperatorTable::Operators() const noexcept
    {
        return m_operators;
    }

} // namespace BootstrapForge::Architecture::Grammar