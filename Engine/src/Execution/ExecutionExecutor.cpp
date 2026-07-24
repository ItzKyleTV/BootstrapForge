#include <BootstrapForge/Engine/Execution/ExecutionExecutor.hpp>

#include <BootstrapForge/Domain/Operations/ExecutionOutcome.hpp>
#include <BootstrapForge/Domain/Operations/ExecutionResult.hpp>
#include <BootstrapForge/Domain/Operations/ExecutionResultEntry.hpp>
#include <BootstrapForge/Domain/Operations/Operation.hpp>

#include <vector>

namespace BootstrapForge::Engine::Execution
{
    Domain::Operations::ExecutionResult
        ExecutionExecutor::Execute(
            const Domain::Operations::ScheduledExecutionPlan& plan,
            Contracts::Execution::IExecutionBackend& backend) const
    {
        using Domain::Operations::ExecutionOutcome;
        using Domain::Operations::ExecutionResult;
        using Domain::Operations::ExecutionResultEntry;
        using Domain::Operations::OperationKind;

        std::vector<ExecutionResultEntry> entries;
        entries.reserve(plan.Operations().size());

        for (const auto& operation : plan.Operations())
        {
            bool succeeded = false;

            switch (operation.Kind())
            {
            case OperationKind::Create:
                succeeded = backend.Create(operation.Resource());
                break;

            case OperationKind::Update:
                succeeded = backend.Update(operation.Resource());
                break;

            case OperationKind::Delete:
                succeeded = backend.Delete(operation.Resource());
                break;

            default:
                break;
            }

            entries.emplace_back(
                operation,
                succeeded
                ? ExecutionOutcome::Success
                : ExecutionOutcome::Failure);
        }

        return ExecutionResult(
            std::move(entries));
    }
}