#include <BootstrapForge/Engine/Reconciliation/ArchitectureReconciler.hpp>

#include <BootstrapForge/Domain/Models/ArchitectureDelta.hpp>
#include <BootstrapForge/Domain/Models/ArchitectureModel.hpp>

#include <BootstrapForge/Domain/Operations/ExecutionPlan.hpp>
#include <BootstrapForge/Domain/Operations/Operation.hpp>
#include <BootstrapForge/Domain/Operations/OperationKind.hpp>

#include <BootstrapForge/Engine/Differ/ArchitectureDiffer.hpp>
#include <BootstrapForge/Engine/Planner/ArchitecturePlanner.hpp>

namespace BootstrapForge::Engine::Reconciliation
{
    Domain::Operations::ExecutionPlan
        ArchitectureReconciler::Reconcile(
            const Domain::Models::ArchitectureModel& desired,
            const Domain::Models::ArchitectureModel& current) const
    {
        Differ::ArchitectureDiffer differ;

        Planner::ArchitecturePlanner planner;

        const Domain::Models::ArchitectureDelta delta =
            differ.Compare(
                desired,
                current);

        return planner.Plan(delta);
    }

} // namespace BootstrapForge::Engine::Reconciliation