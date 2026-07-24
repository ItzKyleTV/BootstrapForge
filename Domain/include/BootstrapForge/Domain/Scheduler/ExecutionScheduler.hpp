#pragma once

#include <BootstrapForge/Domain/Operations/ExecutionPlan.hpp>
#include <BootstrapForge/Domain/Operations/ScheduledExecutionPlan.hpp>

namespace BootstrapForge::Engine::Scheduler
{
    class ExecutionScheduler
    {
    public:
        [[nodiscard]]
        Domain::Operations::ScheduledExecutionPlan Schedule(
            const Domain::Operations::ExecutionPlan& plan) const;
    };
}