#include <BootstrapForge/Architecture/Parsing/TokenStream.hpp>

#include <stdexcept>
#include <string>
#include <utility>

namespace BootstrapForge::Architecture::Parsing
{
    TokenStream::TokenStream(
        const Grammar::Grammar& grammar,
        std::string_view source)
        :
        m_lexer(grammar, source)
    {
        while (true)
        {
            Token token = m_lexer.NextToken();

            m_tokens.emplace_back(
                std::move(token));

            if (m_tokens.back().Is(
                Grammar::TokenKind::EndOfFile))
            {
                break;
            }
        }
    }

    const Token& TokenStream::Current() const noexcept
    {
        return m_tokens[m_position];
    }

    const Token& TokenStream::Peek(
        std::size_t distance) const
    {
        const std::size_t position =
            m_position + distance;

        if (position >= m_tokens.size())
        {
            return m_tokens.back();
        }

        return m_tokens[position];
    }

    bool TokenStream::IsAtEnd() const noexcept
    {
        return Current().Is(
            Grammar::TokenKind::EndOfFile);
    }

    std::size_t TokenStream::Position() const noexcept
    {
        return m_position;
    }

    std::size_t TokenStream::Size() const noexcept
    {
        return m_tokens.size();
    }

    void TokenStream::Advance()
    {
        if (IsAtEnd())
        {
            return;
        }

        ++m_position;
    }

    bool TokenStream::Match(
        Grammar::TokenKind kind) const noexcept
    {
        return Current().Is(kind);
    }

    bool TokenStream::Consume(
        Grammar::TokenKind kind)
    {
        if (!Match(kind))
        {
            return false;
        }

        Advance();

        return true;
    }

    const Token& TokenStream::Expect(
        Grammar::TokenKind kind)
    {
        if (Match(kind))
        {
            const Token& token = Current();

            Advance();

            return token;
        }

        const Token& token = Current();

        throw std::runtime_error(
            "Unexpected token '" +
            token.Lexeme() +
            "'. Expected token kind " +
            std::to_string(
                static_cast<int>(kind)));
    }

    void TokenStream::Ensure(
        std::size_t distance)
    {
        const std::size_t requiredPosition =
            m_position + distance;

        if (requiredPosition < m_tokens.size())
        {
            return;
        }

        while (
            requiredPosition >= m_tokens.size() &&
            !m_tokens.back().Is(
                Grammar::TokenKind::EndOfFile))
        {
            Token token = m_lexer.NextToken();

            m_tokens.emplace_back(
                std::move(token));
        }
    }

} // namespace BootstrapForge::Architecture::Parsing