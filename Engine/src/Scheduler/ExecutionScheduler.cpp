#include <BootstrapForge/Engine/Scheduler/ExecutionScheduler.hpp>

#include <BootstrapForge/Domain/Models/Relationship.hpp>
#include <BootstrapForge/Domain/Operations/Operation.hpp>
#include <BootstrapForge/Domain/Operations/OperationKind.hpp>
#include <BootstrapForge/Domain/Operations/PlannedOperation.hpp>
#include <BootstrapForge/Domain/Operations/RelationshipOperation.hpp>
#include <BootstrapForge/Domain/Types/ResourceId.hpp>

#include <queue>
#include <stdexcept>
#include <unordered_map>
#include <vector>

namespace BootstrapForge::Engine::Scheduler
{
    namespace
    {
        using Operation =
            BootstrapForge::Domain::Operations::Operation;

        using RelationshipOperation =
            BootstrapForge::Domain::Operations::RelationshipOperation;

        using PlannedOperation =
            BootstrapForge::Domain::Operations::PlannedOperation;

        using OperationKind =
            BootstrapForge::Domain::Operations::OperationKind;

        using Relationship =
            BootstrapForge::Domain::Models::Relationship;

        using ResourceId =
            BootstrapForge::Domain::Types::ResourceId;

        using OperationIndex =
            std::unordered_map<
            ResourceId,
            std::size_t>;

        //
        // Deterministic ordering for operations which are
        // simultaneously ready.
        //
        struct ReadyOperationCompare
        {
            const std::vector<Operation>* Operations = nullptr;

            bool operator()(
                std::size_t left,
                std::size_t right) const
            {
                const auto& leftPath =
                    (*Operations)[left]
                    .Resource()
                    .Path()
                    .Value()
                    .generic_string();

                const auto& rightPath =
                    (*Operations)[right]
                    .Resource()
                    .Path()
                    .Value()
                    .generic_string();

                //
                // priority_queue puts the "largest" item first,
                // so reverse the comparison to obtain lexical
                // ascending order.
                //
                return leftPath > rightPath;
            }
        };

        //
        // Performs a topological sort over a set of resource
        // operations.
        //
        // Forward:
        //
        //     Source -> Target
        //
        // means Source executes first.
        //
        // Reverse:
        //
        //     Source -> Target
        //
        // means Target executes first.
        //
        [[nodiscard]]
        std::vector<Operation>
            TopologicalSchedule(
                const std::vector<Operation>& operations,
                const std::vector<Relationship>& relationships,
                bool reverse)
        {
            if (operations.empty())
            {
                return {};
            }

            OperationIndex indices;

            indices.reserve(
                operations.size());

            for (std::size_t index = 0;
                index < operations.size();
                ++index)
            {
                indices.emplace(
                    operations[index]
                    .Resource()
                    .Id(),
                    index);
            }

            //
            // outgoing[A] contains operations which depend on A.
            //
            std::vector<std::vector<std::size_t>>
                outgoing(
                    operations.size());

            std::vector<std::size_t>
                indegree(
                    operations.size(),
                    0);

            for (const auto& relationship :
                relationships)
            {
                const auto sourceIt =
                    indices.find(
                        relationship.Source());

                const auto targetIt =
                    indices.find(
                        relationship.Target());

                //
                // A relationship can reference a resource that
                // isn't participating in this particular operation
                // set. That dependency is outside this graph.
                //
                if (sourceIt == indices.end() ||
                    targetIt == indices.end())
                {
                    continue;
                }

                const std::size_t source =
                    sourceIt->second;

                const std::size_t target =
                    targetIt->second;

                const std::size_t from =
                    reverse
                    ? target
                    : source;

                const std::size_t to =
                    reverse
                    ? source
                    : target;

                //
                // Avoid duplicate dependency edges.
                //
                const auto& edges =
                    outgoing[from];

                bool alreadyExists = false;

                for (const auto existing :
                    edges)
                {
                    if (existing == to)
                    {
                        alreadyExists = true;
                        break;
                    }
                }

                if (alreadyExists)
                {
                    continue;
                }

                outgoing[from].push_back(to);

                ++indegree[to];
            }

            std::priority_queue<
                std::size_t,
                std::vector<std::size_t>,
                ReadyOperationCompare>
                ready(
                    ReadyOperationCompare{
                        &operations });

            for (std::size_t index = 0;
                index < operations.size();
                ++index)
            {
                if (indegree[index] == 0)
                {
                    ready.push(index);
                }
            }

            std::vector<Operation> result;

            result.reserve(
                operations.size());

            std::size_t processed = 0;

            while (!ready.empty())
            {
                const std::size_t current =
                    ready.top();

                ready.pop();

                result.push_back(
                    operations[current]);

                ++processed;

                for (const std::size_t dependent :
                outgoing[current])
                {
                    --indegree[dependent];

                    if (indegree[dependent] == 0)
                    {
                        ready.push(dependent);
                    }
                }
            }

            //
            // A cycle means there is no valid execution order.
            //
            if (processed != operations.size())
            {
                throw std::runtime_error(
                    reverse
                    ? "Manifest relationship graph contains a cycle while scheduling deletions."
                    : "Manifest relationship graph contains a cycle while scheduling creations.");
            }

            return result;
        }
    }

    Domain::Operations::ScheduledExecutionPlan
        ExecutionScheduler::Schedule(
            const Domain::Operations::ExecutionPlan& plan) const
    {
        std::vector<Operation> creates;
        std::vector<Operation> updates;
        std::vector<Operation> deletes;
        std::vector<Operation> others;

        std::vector<RelationshipOperation>
            addedRelationships;

        std::vector<RelationshipOperation>
            removedRelationships;

        std::vector<Relationship>
            createRelationships;

        std::vector<Relationship>
            deleteRelationships;

        creates.reserve(
            plan.Operations().size());

        updates.reserve(
            plan.Operations().size());

        deletes.reserve(
            plan.Operations().size());

        others.reserve(
            plan.Operations().size());

        addedRelationships.reserve(
            plan.Operations().size());

        removedRelationships.reserve(
            plan.Operations().size());

        createRelationships.reserve(
            plan.Operations().size());

        deleteRelationships.reserve(
            plan.Operations().size());

        //
        // Partition planned operations.
        //
        // PlannedOperation is a variant containing either a
        // resource Operation or a RelationshipOperation.
        //

        for (const auto& plannedOperation :
            plan.Operations())
        {
            if (std::holds_alternative<Operation>(
                plannedOperation))
            {
                const auto& operation =
                    std::get<Operation>(
                        plannedOperation);

                switch (operation.Kind())
                {
                case OperationKind::Create:
                    creates.push_back(operation);
                    break;

                case OperationKind::Update:
                    updates.push_back(operation);
                    break;

                case OperationKind::Delete:
                    deletes.push_back(operation);
                    break;

                default:
                    others.push_back(operation);
                    break;
                }

                continue;
            }

            const auto& relationshipOperation =
                std::get<RelationshipOperation>(
                    plannedOperation);

            switch (relationshipOperation.Kind())
            {
            case OperationKind::AddRelationship:
                addedRelationships.push_back(
                    relationshipOperation);

                createRelationships.push_back(
                    relationshipOperation.Relationship());
                break;

            case OperationKind::RemoveRelationship:
                removedRelationships.push_back(
                    relationshipOperation);

                deleteRelationships.push_back(
                    relationshipOperation.Relationship());
                break;

            default:
                break;
            }
        }

        //
        // REMOVE RELATIONSHIPS
        //
        // Relationships must be removed before their resources
        // can safely be deleted.
        //

        std::vector<PlannedOperation>
            scheduled;

        scheduled.reserve(
            plan.Operations().size());

        for (const auto& relationship :
            removedRelationships)
        {
            scheduled.emplace_back(
                relationship);
        }

        //
        // DELETE
        //
        // Relationship:
        //
        //     A -> B
        //
        // Execution:
        //
        //     B
        //     A
        //
        // Dependencies are therefore reversed.
        //

        deletes =
            TopologicalSchedule(
                deletes,
                deleteRelationships,
                true);

        for (const auto& operation :
            deletes)
        {
            scheduled.emplace_back(
                operation);
        }

        //
        // UPDATE
        //

        for (const auto& operation :
            updates)
        {
            scheduled.emplace_back(
                operation);
        }

        //
        // CREATE
        //
        // Relationship:
        //
        //     A -> B
        //
        // Execution:
        //
        //     A
        //     B
        //

        creates =
            TopologicalSchedule(
                creates,
                createRelationships,
                false);

        for (const auto& operation :
            creates)
        {
            scheduled.emplace_back(
                operation);
        }

        //
        // ADD RELATIONSHIPS
        //
        // Resources must exist before their relationships
        // can be established.
        //

        for (const auto& relationship :
            addedRelationships)
        {
            scheduled.emplace_back(
                relationship);
        }

        //
        // Future operation kinds.
        //
        // Preserve them rather than silently discarding them.
        //

        for (const auto& operation :
            others)
        {
            scheduled.emplace_back(
                operation);
        }

        return Domain::Operations::ScheduledExecutionPlan(
            std::move(scheduled));
    }

} // namespace BootstrapForge::Engine::Scheduler