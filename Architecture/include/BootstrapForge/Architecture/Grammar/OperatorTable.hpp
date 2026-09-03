#pragma once

#include <BootstrapForge/Architecture/Grammar/Operator.hpp>

#include <optional>
#include <string_view>
#include <vector>

namespace BootstrapForge::Architecture::Grammar
{
    class OperatorTable final
    {
    public:

        OperatorTable();

        [[nodiscard]]
        std::optional<Operator> Find(
            std::string_view text) const noexcept;

        [[nodiscard]]
        const std::vector<Operator>& Operators() const noexcept;

    private:

        std::vector<Operator> m_operators;
    };

} // namespace BootstrapForge::Architecture::Grammar