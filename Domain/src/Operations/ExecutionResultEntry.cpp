#include <BootstrapForge/Domain/Operations/ExecutionResultEntry.hpp>

#include <utility>

namespace BootstrapForge::Domain::Operations
{
    ExecutionResultEntry::ExecutionResultEntry(
        Domain::Operations::Operation operation,
        ExecutionOutcome outcome,
        std::string message)
        : m_operation(std::move(operation))
        , m_outcome(outcome)
        , m_message(std::move(message))
    {
    }

    const Domain::Operations::Operation&
        ExecutionResultEntry::Operation() const noexcept
    {
        return m_operation;
    }

    ExecutionOutcome
        ExecutionResultEntry::Outcome() const noexcept
    {
        return m_outcome;
    }

    const std::string&
        ExecutionResultEntry::Message() const noexcept
    {
        return m_message;
    }

    bool
        ExecutionResultEntry::Success() const noexcept
    {
        return m_outcome == ExecutionOutcome::Success;
    }
}