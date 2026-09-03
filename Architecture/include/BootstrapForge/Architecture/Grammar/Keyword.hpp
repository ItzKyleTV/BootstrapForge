#pragma once

#include <BootstrapForge/Architecture/Grammar/TokenKind.hpp>

#include <string_view>

namespace BootstrapForge::Architecture::Grammar
{
    class Keyword final
    {
    public:
        constexpr Keyword(
            std::string_view text,
            TokenKind token) noexcept
            :
            m_text(text),
            m_token(token)
        {
        }

        [[nodiscard]]
        constexpr std::string_view Text() const noexcept
        {
            return m_text;
        }

        [[nodiscard]]
        constexpr TokenKind Token() const noexcept
        {
            return m_token;
        }

    private:
        std::string_view m_text;
        TokenKind m_token;
    };

} // namespace BootstrapForge::Architecture::Grammar