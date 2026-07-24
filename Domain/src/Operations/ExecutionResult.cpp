#include <BootstrapForge/Domain/Operations/ExecutionResult.hpp>

#include <algorithm>

namespace BootstrapForge::Domain::Operations
{
    ExecutionResult::ExecutionResult(
        std::vector<ExecutionResultEntry> entries)
        : m_entries(std::move(entries))
    {
    }

    const std::vector<ExecutionResultEntry>&
        ExecutionResult::Entries() const noexcept
    {
        return m_entries;
    }

    std::size_t
        ExecutionResult::Total() const noexcept
    {
        return m_entries.size();
    }

    std::size_t
        ExecutionResult::Successful() const noexcept
    {
        return static_cast<std::size_t>(
            std::count_if(
                m_entries.begin(),
                m_entries.end(),
                [](const ExecutionResultEntry& entry)
                {
                    return entry.Success();
                }));
    }

    std::size_t
        ExecutionResult::Failed() const noexcept
    {
        return Total() - Successful();
    }

    bool
        ExecutionResult::Success() const noexcept
    {
        return Failed() == 0;
    }
}