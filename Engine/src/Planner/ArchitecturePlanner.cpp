#include <BootstrapForge/Engine/Planner/ArchitecturePlanner.hpp>

#include <BootstrapForge/Domain/Models/ResourceKind.hpp>
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

        //
        // A modified symbolic link becomes two physical operations:
        //
        //     DELETE existing symbolic link
        //     CREATE symbolic link with desired target
        //
        // The execution backend intentionally exposes symbolic-link
        // creation and deletion rather than an UpdateSymbolicLink()
        // operation.
        //
        // The delete operation uses the resource path, while the
        // create operation uses the desired symbolic-link target.
        //
        operations.reserve(
            delta.AddedResources().size() +
            delta.ModifiedResources().size() +
            delta.RemovedResources().size() +
            delta.ModifiedResources().size() +
            delta.AddedRelationships().size() +
            delta.RemovedRelationships().size());

        //
        // Added resources.
        //

        for (const auto& resource :
            delta.AddedResources())
        {
            operations.emplace_back(
                Operations::Operation(
                    Operations::OperationKind::Create,
                    resource));
        }

        //
        // Modified resources.
        //
        // Ordinary resources use UPDATE.
        //
        // Symbolic links are replaced because the execution backend
        // has no physical UpdateSymbolicLink() operation.
        //

        for (const auto& resource :
            delta.ModifiedResources())
        {
            if (resource.Kind() ==
                Models::ResourceKind::SymbolicLink)
            {
                operations.emplace_back(
                    Operations::Operation(
                        Operations::OperationKind::Delete,
                        resource));

                operations.emplace_back(
                    Operations::Operation(
                        Operations::OperationKind::Create,
                        resource));

                continue;
            }

            operations.emplace_back(
                Operations::Operation(
                    Operations::OperationKind::Update,
                    resource));
        }

        //
        // Removed resources.
        //

        for (const auto& resource :
            delta.RemovedResources())
        {
            operations.emplace_back(
                Operations::Operation(
                    Operations::OperationKind::Delete,
                    resource));
        }

        //
        // Added relationships.
        //

        for (const auto& relationship :
            delta.AddedRelationships())
        {
            operations.emplace_back(
                Operations::RelationshipOperation(
                    Operations::OperationKind::AddRelationship,
                    relationship));
        }

        //
        // Removed relationships.
        //

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