#pragma once

#include <BootstrapForge/Architecture/Grammar/Keyword.hpp>

#include <optional>
#include <string_view>
#include <vector>

namespace BootstrapForge::Architecture::Grammar
{
    class KeywordTable final
    {
    public:
        KeywordTable();

        [[nodiscard]]
        std::optional<Keyword> Find(
            std::string_view text) const noexcept;

        [[nodiscard]]
        const std::vector<Keyword>&
            Keywords() const noexcept;

    private:
        std::vector<Keyword> m_keywords;
    };

} // namespace BootstrapForge::Architecture::Grammar