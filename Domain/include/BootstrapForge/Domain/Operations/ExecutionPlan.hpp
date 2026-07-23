#pragma once

#include <BootstrapForge/Domain/Operations/Operation.hpp>

#include <compare>
#include <vector>

namespace BootstrapForge::Domain::Operations
{
    class ExecutionPlan
    {
    public:

        ExecutionPlan() = default;

        explicit ExecutionPlan(
            std::vector<Operation> operations);

        [[nodiscard]]
        const std::vector<Operation>& Operations() const noexcept;

        auto operator<=>(const ExecutionPlan&) const = default;

    private:

        std::vector<Operation> m_operations;
    };

    inline ExecutionPlan::ExecutionPlan(
        std::vector<Operation> operations)
        : m_operations(std::move(operations))
    {
    }

    inline const std::vector<Operation>& ExecutionPlan::Operations() const noexcept
    {
        return m_operations;
    }

} // namespace BootstrapForge::Domain::Operations