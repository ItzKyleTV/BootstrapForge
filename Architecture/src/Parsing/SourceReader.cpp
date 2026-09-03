#include <BootstrapForge/Architecture/Parsing/SourceReader.hpp>

namespace BootstrapForge::Architecture::Parsing
{
    SourceReader::SourceReader(
        std::string_view source) noexcept
        :
        m_source(source)
    {
    }

    void SourceReader::Advance() noexcept
    {
        if (IsEnd())
        {
            return;
        }

        const char current = m_source[m_offset];

        ++m_offset;

        if (current == '\n')
        {
            ++m_line;
            m_column = 1;
        }
        else
        {
            ++m_column;
        }
    }

    void SourceReader::Advance(
        std::size_t count) noexcept
    {
        while (count > 0 && !IsEnd())
        {
            Advance();
            --count;
        }
    }

    bool SourceReader::Match(
        std::string_view text) const noexcept
    {
        if (text.empty())
        {
            return true;
        }

        if (m_offset + text.size() > m_source.size())
        {
            return false;
        }

        return m_source.compare(
            m_offset,
            text.size(),
            text) == 0;
    }

    bool SourceReader::Consume(
        std::string_view text) noexcept
    {
        if (!Match(text))
        {
            return false;
        }

        Advance(text.size());

        return true;
    }

} // namespace BootstrapForge::Architecture::Parsing