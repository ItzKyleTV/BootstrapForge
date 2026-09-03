#pragma once

#include <BootstrapForge/Architecture/Parsing/SourceLocation.hpp>

#include <cstddef>
#include <string_view>

namespace BootstrapForge::Architecture::Parsing
{
    class SourceReader final
    {
    public:

        explicit SourceReader(
            std::string_view source) noexcept;

        [[nodiscard]]
        constexpr bool IsEnd() const noexcept
        {
            return m_offset >= m_source.size();
        }

        [[nodiscard]]
        constexpr char Current() const noexcept
        {
            return IsEnd()
                ? '\0'
                : m_source[m_offset];
        }

        [[nodiscard]]
        constexpr char Peek(
            std::size_t distance = 1) const noexcept
        {
            const std::size_t position =
                m_offset + distance;

            return position < m_source.size()
                ? m_source[position]
                : '\0';
        }

        [[nodiscard]]
        constexpr std::string_view Source() const noexcept
        {
            return m_source;
        }

        [[nodiscard]]
        constexpr std::size_t Offset() const noexcept
        {
            return m_offset;
        }

        [[nodiscard]]
        constexpr std::size_t Line() const noexcept
        {
            return m_line;
        }

        [[nodiscard]]
        constexpr std::size_t Column() const noexcept
        {
            return m_column;
        }

        [[nodiscard]]
        constexpr SourceLocation Location() const noexcept
        {
            return SourceLocation(
                m_offset,
                m_line,
                m_column);
        }

        void Advance() noexcept;

        void Advance(
            std::size_t count) noexcept;

        [[nodiscard]]
        bool Match(
            std::string_view text) const noexcept;

        bool Consume(
            std::string_view text) noexcept;

    private:

        std::string_view m_source;

        std::size_t m_offset = 0;

        std::size_t m_line = 1;

        std::size_t m_column = 1;
    };

} // namespace BootstrapForge::Architecture::Parsing