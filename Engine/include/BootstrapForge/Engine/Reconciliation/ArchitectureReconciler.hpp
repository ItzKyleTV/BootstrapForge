#pragma once

#include <BootstrapForge/Domain/Models/ArchitectureModel.hpp>
#include <BootstrapForge/Domain/Operations/ExecutionPlan.hpp>

namespace BootstrapForge::Engine::Reconciliation
{
    class ArchitectureReconciler final
    {
    public:
        ArchitectureReconciler() = default;

        [[nodiscard]]
        Domain::Operations::ExecutionPlan Reconcile(
            const Domain::Models::ArchitectureModel& desired,
            const Domain::Models::ArchitectureModel& current) const;
    };

} // namespace BootstrapForge::Engine::Reconciliation