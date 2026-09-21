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
        std::vector<Operations::PlannedOperation>
            operations;

        //
        // Modified resources can produce either:
        //
        //     UPDATE desired resource
        //
        // or:
        //
        //     DELETE current resource
        //     CREATE desired resource
        //
        // Replacement is required when the physical resource kind
        // changes, or when a symbolic-link target changes.
        //
        operations.reserve(
            delta.AddedResources().size() +
            delta.ModifiedResources().size() * 2 +
            delta.RemovedResources().size() +
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
        // ResourceModification preserves both the physical current
        // resource and the desired resource.
        //
        for (const auto& modification :
            delta.ModifiedResources())
        {
            const auto& current =
                modification.Current();

            const auto& desired =
                modification.Desired();

            //
            // A resource-kind transition cannot be represented by
            // the backend's Update operations.
            //
            // For example:
            //
            //     File -> Directory
            //
            // must become:
            //
            //     DELETE current file
            //     CREATE desired directory
            //
            const bool kindChanged =
                current.Kind() !=
                desired.Kind();

            //
            // Symbolic links do not have a physical update operation.
            //
            // A target change therefore becomes:
            //
            //     DELETE current symbolic link
            //     CREATE desired symbolic link
            //
            const bool symbolicLinkReplacement =
                current.Kind() ==
                Models::ResourceKind::SymbolicLink &&
                desired.Kind() ==
                Models::ResourceKind::SymbolicLink;

            if (kindChanged ||
                symbolicLinkReplacement)
            {
                operations.emplace_back(
                    Operations::Operation(
                        Operations::OperationKind::Delete,
                        current));

                operations.emplace_back(
                    Operations::Operation(
                        Operations::OperationKind::Create,
                        desired));

                continue;
            }

            //
            // Same-kind ordinary resource modification.
            //
            // Currently this is primarily a file content update.
            //
            operations.emplace_back(
                Operations::Operation(
                    Operations::OperationKind::Update,
                    desired));
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