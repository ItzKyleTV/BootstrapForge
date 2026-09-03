#include <BootstrapForge/Engine/Execution/ExecutionExecutor.hpp>

#include <BootstrapForge/Domain/Models/ResourceKind.hpp>
#include <BootstrapForge/Domain/Operations/ExecutionOutcome.hpp>
#include <BootstrapForge/Domain/Operations/ExecutionResult.hpp>
#include <BootstrapForge/Domain/Operations/ExecutionResultEntry.hpp>
#include <BootstrapForge/Domain/Operations/Operation.hpp>
#include <BootstrapForge/Domain/Operations/OperationKind.hpp>
#include <BootstrapForge/Domain/Operations/RelationshipOperation.hpp>

#include <type_traits>
#include <utility>
#include <variant>
#include <vector>

namespace BootstrapForge::Engine::Execution
{
    using Domain::Models::ResourceKind;

    using Domain::Operations::ExecutionOutcome;
    using Domain::Operations::ExecutionResult;
    using Domain::Operations::ExecutionResultEntry;
    using Domain::Operations::Operation;
    using Domain::Operations::OperationKind;
    using Domain::Operations::RelationshipOperation;

    bool ExecutionExecutor::ExecuteCreate(
        const Operation& operation,
        Contracts::Execution::IExecutionBackend& backend)
    {
        switch (operation.Resource().Kind())
        {
        case ResourceKind::Directory:
            return backend.CreateDirectory(
                operation.Resource());

        case ResourceKind::File:
            return backend.CreateFile(
                operation.Resource());

        case ResourceKind::SymbolicLink:
            return backend.CreateSymbolicLink(
                operation.Resource());
        }

        return false;
    }

    bool ExecutionExecutor::ExecuteUpdate(
        const Operation& operation,
        Contracts::Execution::IExecutionBackend& backend)
    {
        switch (operation.Resource().Kind())
        {
        case ResourceKind::File:
            return backend.UpdateFile(
                operation.Resource());

        case ResourceKind::Directory:
        case ResourceKind::SymbolicLink:
            return false;
        }

        return false;
    }

    bool ExecutionExecutor::ExecuteDelete(
        const Operation& operation,
        Contracts::Execution::IExecutionBackend& backend)
    {
        switch (operation.Resource().Kind())
        {
        case ResourceKind::Directory:
            return backend.DeleteDirectory(
                operation.Resource());

        case ResourceKind::File:
            return backend.DeleteFile(
                operation.Resource());

        case ResourceKind::SymbolicLink:
            return backend.DeleteSymbolicLink(
                operation.Resource());
        }

        return false;
    }

    bool ExecutionExecutor::ExecuteRelationship(
        const RelationshipOperation& operation,
        Contracts::Execution::IExecutionBackend& backend)
    {
        switch (operation.Kind())
        {
        case OperationKind::AddRelationship:
            return backend.AddRelationship(
                operation.Relationship());

        case OperationKind::RemoveRelationship:
            return backend.RemoveRelationship(
                operation.Relationship());

        default:
            return false;
        }
    }

    ExecutionResult
        ExecutionExecutor::Execute(
            const Domain::Operations::ScheduledExecutionPlan& plan,
            Contracts::Execution::IExecutionBackend& backend) const
    {
        std::vector<ExecutionResultEntry> entries;

        entries.reserve(
            plan.Operations().size());

        for (const auto& plannedOperation :
            plan.Operations())
        {
            bool succeeded = false;

            std::visit(
                [&](const auto& operation)
                {
                    using OperationType =
                        std::decay_t<decltype(operation)>;

                    if constexpr (
                        std::is_same_v<
                        OperationType,
                        Operation>)
                    {
                        switch (operation.Kind())
                        {
                        case OperationKind::Create:
                            succeeded =
                                ExecuteCreate(
                                    operation,
                                    backend);
                            break;

                        case OperationKind::Update:
                            succeeded =
                                ExecuteUpdate(
                                    operation,
                                    backend);
                            break;

                        case OperationKind::Delete:
                            succeeded =
                                ExecuteDelete(
                                    operation,
                                    backend);
                            break;

                        default:
                            succeeded = false;
                            break;
                        }
                    }
                    else if constexpr (
                        std::is_same_v<
                        OperationType,
                        RelationshipOperation>)
                    {
                        succeeded =
                            ExecuteRelationship(
                                operation,
                                backend);
                    }
                },
                plannedOperation);

            entries.emplace_back(
                plannedOperation,
                succeeded
                ? ExecutionOutcome::Success
                : ExecutionOutcome::Failure);
        }

        return ExecutionResult(
            std::move(entries));
    }

} // namespace BootstrapForge::Engine::Execution