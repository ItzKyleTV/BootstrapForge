#include <BootstrapForge/Application/Identity/ResourceFactory.hpp>
#include <BootstrapForge/Application/Identity/RelationshipFactory.hpp>
#include <BootstrapForge/Application/Mapping/ManifestMapper.hpp>

#include <BootstrapForge/Domain/Models/ArchitectureModel.hpp>

#include <BootstrapForge/Domain/Operations/ExecutionPlan.hpp>
#include <BootstrapForge/Domain/Operations/ExecutionResult.hpp>
#include <BootstrapForge/Domain/Operations/ExecutionResultEntry.hpp>
#include <BootstrapForge/Domain/Operations/Operation.hpp>
#include <BootstrapForge/Domain/Operations/OperationKind.hpp>
#include <BootstrapForge/Domain/Operations/RelationshipOperation.hpp>
#include <BootstrapForge/Domain/Operations/ScheduledExecutionPlan.hpp>

#include <BootstrapForge/Engine/Execution/ExecutionExecutor.hpp>
#include <BootstrapForge/Engine/Reconciliation/ArchitectureReconciler.hpp>
#include <BootstrapForge/Engine/Scheduler/ExecutionScheduler.hpp>

#include <BootstrapForge/Infrastructure/Cryptography/Hashing/Blake3Hasher.hpp>

#include <BootstrapForge/Infrastructure/Deserialization/JsonManifestDeserializer.hpp>

#include <BootstrapForge/Infrastructure/Execution/PhysicalExecutionBackend.hpp>

#include <BootstrapForge/Infrastructure/Filesystem/ManifestFileReader.hpp>
#include <BootstrapForge/Infrastructure/Filesystem/PhysicalFilesystemScanner.hpp>

#include <BootstrapForge/Infrastructure/Loading/JsonDesiredArchitectureLoader.hpp>

#include <BootstrapForge/Runtime/RuntimeOptions.hpp>
#include <BootstrapForge/Runtime/RuntimeWorkspace.hpp>

#include <cstdlib>
#include <iostream>
#include <type_traits>
#include <variant>

using namespace BootstrapForge;

namespace
{
    const char* OperationKindName(
        Domain::Operations::OperationKind kind) noexcept
    {
        using Domain::Operations::OperationKind;

        switch (kind)
        {
        case OperationKind::Create:
            return "CREATE";

        case OperationKind::Delete:
            return "DELETE";

        case OperationKind::Update:
            return "UPDATE";

        case OperationKind::Move:
            return "MOVE";

        case OperationKind::Rename:
            return "RENAME";

        case OperationKind::AddRelationship:
            return "ADD_RELATIONSHIP";

        case OperationKind::RemoveRelationship:
            return "REMOVE_RELATIONSHIP";

        default:
            return "UNKNOWN";
        }
    }

    void DumpModel(
        const char* name,
        const Domain::Models::ArchitectureModel& model)
    {
        std::cout << '\n';
        std::cout << "===== " << name << " =====\n";

        for (const auto& resource : model.Resources())
        {
            std::cout
                << "Kind: "
                << static_cast<int>(resource.Kind())
                << "  Path: "
                << resource.Path().ToString()
                << '\n';
        }

        std::cout
            << "Relationships: "
            << model.Relationships().size()
            << '\n';

        std::cout
            << "=========================\n";
    }

    void PrintExecutionEntry(
        const Domain::Operations::ExecutionResultEntry& entry)
    {
        std::cout
            << (entry.Success()
                ? "[OK]   "
                : "[FAIL] ");

        std::visit(
            [](const auto& operation)
            {
                using OperationType =
                    std::decay_t<decltype(operation)>;

                if constexpr (
                    std::is_same_v<
                    OperationType,
                    Domain::Operations::Operation>)
                {
                    std::cout
                        << OperationKindName(
                            operation.Kind())
                        << "  "
                        << operation.Resource()
                        .Path()
                        .ToString();
                }
                else if constexpr (
                    std::is_same_v<
                    OperationType,
                    Domain::Operations::RelationshipOperation>)
                {
                    std::cout
                        << OperationKindName(
                            operation.Kind())
                        << "  "
                        << "relationship";
                }
            },
            entry.Operation());

        if (!entry.Message().empty())
        {
            std::cout
                << "  "
                << entry.Message();
        }

        std::cout << '\n';
    }
}

int main(
    int argc,
    char* argv[])
{
    Runtime::RuntimeOptions options(
        argc,
        argv);

    Runtime::RuntimeWorkspace workspace;

    workspace.EnsureExists();

    //
    // Composition Root
    //

    Infrastructure::Cryptography::Hashing::Blake3Hasher hasher;

    //
    // Resource identity factory.
    //

    Application::Identity::ResourceFactory resourceFactory(
        hasher);

    //
    // Relationship identity factory.
    //

    Application::Identity::RelationshipFactory relationshipFactory(
        hasher);

    //
    // Manifest mapper.
    //

    Application::Mapping::ManifestMapper mapper(
        resourceFactory,
        relationshipFactory);

    Infrastructure::Filesystem::ManifestFileReader fileReader;

    Infrastructure::Deserialization::JsonManifestDeserializer
        deserializer;

    Infrastructure::Loading::JsonDesiredArchitectureLoader loader(
        fileReader,
        deserializer,
        mapper);

    //
    // Load desired architecture.
    //

    const Domain::Models::ArchitectureModel desired =
        loader.Load(
            workspace.Desired() /
            "bootstrapforge.json");

    //
    // Discover current architecture.
    //

    Infrastructure::Filesystem::PhysicalFilesystemScanner scanner;

    const Domain::Models::ArchitectureModel current =
        scanner.Scan(
            workspace.Current());

    //
    // Diagnostic model output.
    //

    DumpModel(
        "DESIRED MODEL",
        desired);

    DumpModel(
        "CURRENT MODEL",
        current);

    //
    // Reconciliation pipeline.
    //
    // ArchitectureReconciler owns the:
    //
    //     ArchitectureDiffer
    //              |
    //     ArchitectureDelta
    //              |
    //     ArchitecturePlanner
    //              |
    //        ExecutionPlan
    //
    // main() therefore does not need to know
    // how reconciliation is internally performed.
    //

    Engine::Reconciliation::ArchitectureReconciler reconciler;

    const Domain::Operations::ExecutionPlan plan =
        reconciler.Reconcile(
            desired,
            current);

    //
    // Schedule execution.
    //

    Engine::Scheduler::ExecutionScheduler scheduler;

    const Domain::Operations::ScheduledExecutionPlan scheduledPlan =
        scheduler.Schedule(
            plan);

    //
    // Execute.
    //

    Engine::Execution::ExecutionExecutor executor;

    Infrastructure::Execution::PhysicalExecutionBackend backend(
        workspace.Current());

    const Domain::Operations::ExecutionResult result =
        executor.Execute(
            scheduledPlan,
            backend);

    //
    // Output.
    //

    std::cout
        << '\n'
        << "Workspace : "
        << workspace.Root().generic_string()
        << '\n';

    std::cout
        << "Scan Root : "
        << workspace.Current().generic_string()
        << "\n\n";

    std::cout
        << "Executed Operations\n";

    std::cout
        << "-------------------\n";

    for (const auto& entry :
        result.Entries())
    {
        PrintExecutionEntry(
            entry);
    }

    std::cout << '\n';

    std::cout
        << "Total      : "
        << result.Total()
        << '\n';

    std::cout
        << "Successful : "
        << result.Successful()
        << '\n';

    std::cout
        << "Failed     : "
        << result.Failed()
        << '\n';

    return result.Success()
        ? EXIT_SUCCESS
        : EXIT_FAILURE;
}