#pragma once

#include <BootstrapForge/Architecture/Parsing/Lexer.hpp>
#include <BootstrapForge/Architecture/Parsing/Token.hpp>

#include <cstddef>
#include <string_view>
#include <vector>

namespace BootstrapForge::Architecture::Parsing
{
    class TokenStream final
    {
    public:

        TokenStream(
            const Grammar::Grammar& grammar,
            std::string_view source);

        [[nodiscard]]
        const Token& Current() const noexcept;

        [[nodiscard]]
        const Token& Peek(
            std::size_t distance = 0) const;

        [[nodiscard]]
        bool IsAtEnd() const noexcept;

        [[nodiscard]]
        std::size_t Position() const noexcept;

        [[nodiscard]]
        std::size_t Size() const noexcept;

        void Advance();

        [[nodiscard]]
        bool Match(
            Grammar::TokenKind kind) const noexcept;

        [[nodiscard]]
        bool Consume(
            Grammar::TokenKind kind);

        const Token& Expect(
            Grammar::TokenKind kind);

    private:

        void Ensure(
            std::size_t distance);

        std::vector<Token> m_tokens;

        std::size_t m_position = 0;

        Lexer m_lexer;
    };

} // namespace BootstrapForge::Architecture::Parsing