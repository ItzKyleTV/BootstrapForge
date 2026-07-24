#include <BootstrapForge/Engine/Planner/ArchitecturePlanner.hpp>

#include <vector>

namespace BootstrapForge::Engine::Planner
{
    using namespace BootstrapForge::Domain;

    Operations::ExecutionPlan ArchitecturePlanner::Plan(
        const Models::ArchitectureDelta& delta) const
    {
        std::vector<Operations::Operation> operations;

        //
        // Create operations
        //
        for (const auto& resource : delta.AddedResources())
        {
            operations.emplace_back(
                Operations::OperationKind::Create,
                resource);
        }

        //
        // Update operations
        //
        for (const auto& resource : delta.ModifiedResources())
        {
            operations.emplace_back(
                Operations::OperationKind::Update,
                resource);
        }

        //
        // Delete operations
        //
        for (const auto& resource : delta.RemovedResources())
        {
            operations.emplace_back(
                Operations::OperationKind::Delete,
                resource);
        }

        return Operations::ExecutionPlan(
            std::move(operations));
    }
}