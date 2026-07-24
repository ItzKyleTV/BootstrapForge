#pragma once

#include <BootstrapForge/Domain/Operations/ExecutionResultEntry.hpp>

#include <vector>

namespace BootstrapForge::Domain::Operations
{
    class ExecutionResult
    {
    public:

        explicit ExecutionResult(
            std::vector<ExecutionResultEntry> entries);

        [[nodiscard]]
        const std::vector<ExecutionResultEntry>& Entries() const noexcept;

        [[nodiscard]]
        std::size_t Total() const noexcept;

        [[nodiscard]]
        std::size_t Successful() const noexcept;

        [[nodiscard]]
        std::size_t Failed() const noexcept;

        [[nodiscard]]
        bool Success() const noexcept;

    private:

        std::vector<ExecutionResultEntry> m_entries;
    };
}