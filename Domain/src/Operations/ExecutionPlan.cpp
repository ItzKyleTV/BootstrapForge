#include <BootstrapForge/Domain/Operations/ExecutionPlan.hpp>

#include <utility>

namespace BootstrapForge::Domain::Operations
{
    ExecutionPlan::ExecutionPlan(
        std::vector<PlannedOperation> operations)
        :
        m_operations(std::move(operations))
    {
    }

    const std::vector<PlannedOperation>&
        ExecutionPlan::Operations() const noexcept
    {
        return m_operations;
    }

} // namespace BootstrapForge::Domain::Operations