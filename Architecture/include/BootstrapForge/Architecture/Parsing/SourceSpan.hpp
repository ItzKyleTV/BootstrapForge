#pragma once

#include <BootstrapForge/Architecture/Parsing/SourceLocation.hpp>

#include <compare>
#include <cstddef>

namespace BootstrapForge::Architecture::Parsing
{
    class SourceSpan final
    {
    public:

        constexpr SourceSpan() noexcept = default;

        constexpr SourceSpan(
            SourceLocation begin,
            SourceLocation end) noexcept
            :
            m_begin(begin),
            m_end(end)
        {
        }

        [[nodiscard]]
        constexpr const SourceLocation& Begin() const noexcept
        {
            return m_begin;
        }

        [[nodiscard]]
        constexpr const SourceLocation& End() const noexcept
        {
            return m_end;
        }

        [[nodiscard]]
        constexpr std::size_t Length() const noexcept
        {
            return
                (m_end.Offset >= m_begin.Offset)
                ? (m_end.Offset - m_begin.Offset)
                : 0;
        }

        [[nodiscard]]
        constexpr bool Empty() const noexcept
        {
            return Length() == 0;
        }

        [[nodiscard]]
        constexpr bool Contains(
            const SourceLocation& location) const noexcept
        {
            return
                location.Offset >= m_begin.Offset &&
                location.Offset <= m_end.Offset;
        }

        [[nodiscard]]
        constexpr auto operator<=>(const SourceSpan&) const noexcept = default;

    private:

        SourceLocation m_begin;

        SourceLocation m_end;
    };

} // namespace BootstrapForge::Architecture::Parsing