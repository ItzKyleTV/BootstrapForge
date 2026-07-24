#pragma once

#include <BootstrapForge/Domain/Operations/Operation.hpp>

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
            std::vector<Operation> operations)
            :
            m_operations(std::move(operations))
        {
        }

        [[nodiscard]]
        const std::vector<Operation>& Operations() const noexcept
        {
            return m_operations;
        }

        auto operator<=>(const ScheduledExecutionPlan&) const = default;

    private:
        std::vector<Operation> m_operations;
    };
}