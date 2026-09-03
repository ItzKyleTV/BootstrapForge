#include <BootstrapForge/Domain/Operations/ExecutionResultEntry.hpp>

namespace BootstrapForge::Domain::Operations
{
    ExecutionResultEntry::ExecutionResultEntry(
        PlannedOperation operation,
        ExecutionOutcome outcome,
        std::string message)
        :
        m_operation(std::move(operation)),
        m_outcome(outcome),
        m_message(std::move(message))
    {
    }

} // namespace BootstrapForge::Domain::Operations