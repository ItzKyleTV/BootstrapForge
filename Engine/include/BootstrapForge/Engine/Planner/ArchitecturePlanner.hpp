#pragma once

#include <BootstrapForge/Domain/Models/ArchitectureDelta.hpp>
#include <BootstrapForge/Domain/Operations/ExecutionPlan.hpp>

namespace BootstrapForge::Engine::Planner
{
    class ArchitecturePlanner
    {
    public:

        [[nodiscard]]
        BootstrapForge::Domain::Operations::ExecutionPlan Plan(
            const BootstrapForge::Domain::Models::ArchitectureDelta& delta) const;
    };

} // namespace BootstrapForge::Engine::Planner