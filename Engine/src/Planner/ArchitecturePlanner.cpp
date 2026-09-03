#include <BootstrapForge/Engine/Planner/ArchitecturePlanner.hpp>

#include <BootstrapForge/Domain/Operations/Operation.hpp>
#include <BootstrapForge/Domain/Operations/OperationKind.hpp>
#include <BootstrapForge/Domain/Operations/RelationshipOperation.hpp>

#include <utility>
#include <vector>

namespace BootstrapForge::Engine::Planner
{
    using namespace BootstrapForge::Domain;

    Operations::ExecutionPlan
        ArchitecturePlanner::Plan(
            const Models::ArchitectureDelta& delta) const
    {
        std::vector<Operations::PlannedOperation> operations;

        operations.reserve(
            delta.AddedResources().size() +
            delta.ModifiedResources().size() +
            delta.RemovedResources().size() +
            delta.AddedRelationships().size() +
            delta.RemovedRelationships().size());

        for (const auto& resource :
            delta.AddedResources())
        {
            operations.emplace_back(
                Operations::Operation(
                    Operations::OperationKind::Create,
                    resource));
        }

        for (const auto& resource :
            delta.ModifiedResources())
        {
            operations.emplace_back(
                Operations::Operation(
                    Operations::OperationKind::Update,
                    resource));
        }

        for (const auto& resource :
            delta.RemovedResources())
        {
            operations.emplace_back(
                Operations::Operation(
                    Operations::OperationKind::Delete,
                    resource));
        }

        for (const auto& relationship :
            delta.AddedRelationships())
        {
            operations.emplace_back(
                Operations::RelationshipOperation(
                    Operations::OperationKind::AddRelationship,
                    relationship));
        }

        for (const auto& relationship :
            delta.RemovedRelationships())
        {
            operations.emplace_back(
                Operations::RelationshipOperation(
                    Operations::OperationKind::RemoveRelationship,
                    relationship));
        }

        return Operations::ExecutionPlan(
            std::move(operations));
    }

} // namespace BootstrapForge::Engine::Planner