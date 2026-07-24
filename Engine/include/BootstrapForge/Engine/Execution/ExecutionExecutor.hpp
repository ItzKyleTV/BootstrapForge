#pragma once

#include <BootstrapForge/Contracts/Execution/IExecutionBackend.hpp>

#include <BootstrapForge/Domain/Operations/ExecutionResult.hpp>
#include <BootstrapForge/Domain/Operations/ScheduledExecutionPlan.hpp>

namespace BootstrapForge::Engine::Execution
{
    class ExecutionExecutor
    {
    public:

        [[nodiscard]]
        Domain::Operations::ExecutionResult Execute(
            const Domain::Operations::ScheduledExecutionPlan& plan,
            Contracts::Execution::IExecutionBackend& backend) const;
    };
}