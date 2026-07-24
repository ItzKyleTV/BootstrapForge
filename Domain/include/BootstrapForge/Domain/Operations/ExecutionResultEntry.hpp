#pragma once

#include <BootstrapForge/Domain/Operations/ExecutionOutcome.hpp>
#include <BootstrapForge/Domain/Operations/Operation.hpp>

#include <string>

namespace BootstrapForge::Domain::Operations
{
    class ExecutionResultEntry
    {
    public:

        ExecutionResultEntry(
            Operation operation,
            ExecutionOutcome outcome,
            std::string message = {});

        [[nodiscard]]
        const Operation& Operation() const noexcept;

        [[nodiscard]]
        ExecutionOutcome Outcome() const noexcept;

        [[nodiscard]]
        const std::string& Message() const noexcept;

        [[nodiscard]]
        bool Success() const noexcept;

    private:

        Domain::Operations::Operation m_operation;
        ExecutionOutcome m_outcome;
        std::string m_message;
    };
}