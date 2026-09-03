#pragma once

#include <BootstrapForge/Domain/Operations/PlannedOperation.hpp>

#include <compare>
#include <utility>
#include <vector>

namespace BootstrapForge::Domain::Operations
{
    class ScheduledExecutionPlan
    {
    public:

        ScheduledExecutionPlan() = default;

        explicit ScheduledExecutionPlan(
            std::vector<PlannedOperation> operations)
            :
            m_operations(std::move(operations))
        {
        }

        [[nodiscard]]
        const std::vector<PlannedOperation>&
            Operations() const noexcept
        {
            return m_operations;
        }

        auto operator<=>(
            const ScheduledExecutionPlan&) const = default;

    private:

        std::vector<PlannedOperation> m_operations;
    };

} // namespace BootstrapForge::Domain::Operations