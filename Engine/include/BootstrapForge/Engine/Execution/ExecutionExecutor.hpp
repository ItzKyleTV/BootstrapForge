#pragma once

#include <BootstrapForge/Contracts/Execution/IExecutionBackend.hpp>

#include <BootstrapForge/Domain/Operations/ExecutionResult.hpp>
#include <BootstrapForge/Domain/Operations/Operation.hpp>
#include <BootstrapForge/Domain/Operations/RelationshipOperation.hpp>
#include <BootstrapForge/Domain/Operations/ScheduledExecutionPlan.hpp>

namespace BootstrapForge::Engine::Execution
{
    class ExecutionExecutor
    {
    public:

        [[nodiscard]]
        Domain::Operations::ExecutionResult Execute(
            const Domain::Operations::ScheduledExecutionPlan& plan,
            Contracts::Execution::IExecutionBackend& backend) const;

    private:

        [[nodiscard]]
        static bool ExecuteCreate(
            const Domain::Operations::Operation& operation,
            Contracts::Execution::IExecutionBackend& backend);

        [[nodiscard]]
        static bool ExecuteUpdate(
            const Domain::Operations::Operation& operation,
            Contracts::Execution::IExecutionBackend& backend);

        [[nodiscard]]
        static bool ExecuteDelete(
            const Domain::Operations::Operation& operation,
            Contracts::Execution::IExecutionBackend& backend);

        [[nodiscard]]
        static bool ExecuteRelationship(
            const Domain::Operations::RelationshipOperation& operation,
            Contracts::Execution::IExecutionBackend& backend);
    };
}