#pragma once

#include <compare>
#include <cstddef>

namespace BootstrapForge::Architecture::Parsing
{
    class SourceLocation final
    {
    public:

        constexpr SourceLocation() noexcept = default;

        constexpr SourceLocation(
            std::size_t offset,
            std::size_t line,
            std::size_t column) noexcept
            :
            Offset(offset),
            Line(line),
            Column(column)
        {
        }

        [[nodiscard]]
        constexpr bool IsValid() const noexcept
        {
            return Line > 0 && Column > 0;
        }

        [[nodiscard]]
        constexpr auto operator<=>(const SourceLocation&) const noexcept = default;

        std::size_t Offset = 0;

        std::size_t Line = 1;

        std::size_t Column = 1;
    };

} // namespace BootstrapForge::Architecture::Parsing