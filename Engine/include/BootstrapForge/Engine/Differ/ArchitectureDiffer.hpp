#pragma once

#include <BootstrapForge/Domain/Models/ArchitectureModel.hpp>
#include <BootstrapForge/Domain/Operations/ExecutionPlan.hpp>

namespace BootstrapForge::Engine::Differ
{

    class ArchitectureDiffer
    {
    public:

        [[nodiscard]]
        BootstrapForge::Domain::Operations::ExecutionPlan Compare(
            const BootstrapForge::Domain::Models::ArchitectureModel& desired,
            const BootstrapForge::Domain::Models::ArchitectureModel& current) const;
    };

}