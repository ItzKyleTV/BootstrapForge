#pragma once

#include <BootstrapForge/Domain/Operations/ExecutionOutcome.hpp>
#include <BootstrapForge/Domain/Operations/PlannedOperation.hpp>

#include <string>
#include <utility>

namespace BootstrapForge::Domain::Operations
{
    class ExecutionResultEntry
    {
    public:

        ExecutionResultEntry(
            PlannedOperation operation,
            ExecutionOutcome outcome,
            std::string message = {});

        [[nodiscard]]
        const PlannedOperation&
            Operation() const noexcept
        {
            return m_operation;
        }

        [[nodiscard]]
        ExecutionOutcome
            Outcome() const noexcept
        {
            return m_outcome;
        }

        [[nodiscard]]
        const std::string&
            Message() const noexcept
        {
            return m_message;
        }

        [[nodiscard]]
        bool Success() const noexcept
        {
            return m_outcome == ExecutionOutcome::Success;
        }

    private:

        PlannedOperation m_operation;

        ExecutionOutcome m_outcome =
            ExecutionOutcome::Success;

        std::string m_message;
    };

} // namespace BootstrapForge::Domain::Operations