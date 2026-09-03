#pragma once

#include <BootstrapForge/Architecture/Grammar/TokenKind.hpp>

#include <compare>
#include <string_view>

namespace BootstrapForge::Architecture::Grammar
{
    class Operator final
    {
    public:

        constexpr Operator(
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

        [[nodiscard]]
        constexpr bool IsUnknown() const noexcept
        {
            return m_token == TokenKind::Unknown;
        }

        constexpr auto operator<=>(
            const Operator&) const noexcept = default;

    private:

        std::string_view m_text;

        TokenKind m_token;
    };

} // namespace BootstrapForge::Architecture::Grammar