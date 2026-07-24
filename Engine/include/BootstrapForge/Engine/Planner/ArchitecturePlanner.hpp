#pragma once

#include <BootstrapForge/Domain/Models/ArchitectureDelta.hpp>
#include <BootstrapForge/Domain/Operations/ExecutionPlan.hpp>

namespace BootstrapForge::Engine::Planner
{
    class ArchitecturePlanner
    {
    public:

        [[nodiscard]]
        Domain::Operations::ExecutionPlan Plan(
            const Domain::Models::ArchitectureDelta& delta) const;
    };
}