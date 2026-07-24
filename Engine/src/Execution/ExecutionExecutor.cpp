#include <BootstrapForge/Engine/Execution/ExecutionExecutor.hpp>

#include <BootstrapForge/Domain/Operations/Operation.hpp>

namespace BootstrapForge::Engine::Execution
{
    void ExecutionExecutor::Execute(
        const Domain::Operations::ScheduledExecutionPlan& plan,
        Contracts::Execution::IExecutionBackend& backend) const
    {
        using Domain::Operations::OperationKind;

        for (const auto& operation : plan.Operations())
        {
            switch (operation.Kind())
            {
            case OperationKind::Create:
                backend.Create(operation.Resource());
                break;

            case OperationKind::Update:
                backend.Update(operation.Resource());
                break;

            case OperationKind::Delete:
                backend.Delete(operation.Resource());
                break;

            default:
                break;
            }
        }
    }
}