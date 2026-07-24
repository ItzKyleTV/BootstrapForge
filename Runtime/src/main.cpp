#include <BootstrapForge/Application/Identity/ResourceFactory.hpp>

#include <BootstrapForge/Domain/Models/ArchitectureDelta.hpp>
#include <BootstrapForge/Domain/Models/ArchitectureModel.hpp>
#include <BootstrapForge/Domain/Operations/ExecutionPlan.hpp>
#include <BootstrapForge/Domain/Operations/ExecutionResult.hpp>
#include <BootstrapForge/Domain/Operations/ScheduledExecutionPlan.hpp>

#include <BootstrapForge/Engine/Differ/ArchitectureDiffer.hpp>
#include <BootstrapForge/Engine/Execution/ExecutionExecutor.hpp>
#include <BootstrapForge/Engine/Planner/ArchitecturePlanner.hpp>
#include <BootstrapForge/Engine/Scheduler/ExecutionScheduler.hpp>

#include <BootstrapForge/Infrastructure/Cryptography/Hashing/Blake3Hasher.hpp>
#include <BootstrapForge/Infrastructure/Execution/NullExecutionBackend.hpp>

#include <iostream>

using namespace BootstrapForge;

int main()
{
    Infrastructure::Cryptography::Hashing::Blake3Hasher hasher;

    Application::Identity::ResourceFactory resourceFactory(hasher);

    const Domain::Models::ArchitectureModel desired(
        {
            resourceFactory.Create("/"),
            resourceFactory.Create("/src"),
            resourceFactory.Create("/src/main.cpp")
        },
        {});

    const Domain::Models::ArchitectureModel current;

    Engine::Differ::ArchitectureDiffer differ;
    Engine::Planner::ArchitecturePlanner planner;
    Engine::Scheduler::ExecutionScheduler scheduler;
    Engine::Execution::ExecutionExecutor executor;

    const Domain::Models::ArchitectureDelta delta =
        differ.Compare(desired, current);

    const Domain::Operations::ExecutionPlan plan =
        planner.Plan(delta);

    const Domain::Operations::ScheduledExecutionPlan scheduledPlan =
        scheduler.Schedule(plan);

    Infrastructure::Execution::NullExecutionBackend backend;

    const Domain::Operations::ExecutionResult result =
        executor.Execute(
            scheduledPlan,
            backend);

    std::cout << "Executed Operations\n";
    std::cout << "-------------------\n";

    for (const auto& entry : result.Entries())
    {
        std::cout
            << (entry.Success() ? "[OK]   " : "[FAIL] ")
            << entry.Operation().Resource().Path().Value().generic_string()
            << '\n';
    }

    std::cout << '\n';

    std::cout
        << "Total      : " << result.Total() << '\n'
        << "Successful : " << result.Successful() << '\n'
        << "Failed     : " << result.Failed() << '\n';

    return result.Success() ? 0 : 1;
}