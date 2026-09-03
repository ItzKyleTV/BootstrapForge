#pragma once

#include <BootstrapForge/Domain/Operations/PlannedOperation.hpp>

#include <compare>
#include <utility>
#include <vector>

namespace BootstrapForge::Domain::Operations
{
    class ExecutionPlan
    {
    public:

        ExecutionPlan() = default;

        explicit ExecutionPlan(
            std::vector<PlannedOperation> operations);

        [[nodiscard]]
        const std::vector<PlannedOperation>&
            Operations() const noexcept;

        auto operator<=>(const ExecutionPlan&) const = default;

    private:

        std::vector<PlannedOperation> m_operations;
    };

} // namespace BootstrapForge::Domain::Operations
