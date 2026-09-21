#include <BootstrapForge/Application/Identity/RelationshipFactory.hpp>
#include <BootstrapForge/Application/Identity/ResourceFactory.hpp>
#include <BootstrapForge/Application/Mapping/ManifestMapper.hpp>

#include <BootstrapForge/Domain/Models/ArchitectureModel.hpp>
#include <BootstrapForge/Domain/Operations/ExecutionPlan.hpp>
#include <BootstrapForge/Domain/Operations/ExecutionResult.hpp>
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

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <iostream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <system_error>

namespace
{
    using namespace BootstrapForge;

    class TestWorkspace final
    {
    public:

        TestWorkspace()
        {
            const auto base =
                std::filesystem::temp_directory_path();

            m_root =
                base /
                "BootstrapForge.ReconciliationIntegrationTests";

            std::error_code error;

            std::filesystem::remove_all(
                m_root,
                error);

            std::filesystem::create_directories(
                m_root / "desired",
                error);

            if (error)
            {
                throw std::runtime_error(
                    "Failed to create integration-test workspace: " +
                    error.message());
            }

            std::filesystem::create_directories(
                m_root / "current",
                error);

            if (error)
            {
                throw std::runtime_error(
                    "Failed to create integration-test current directory: " +
                    error.message());
            }
        }

        ~TestWorkspace()
        {
            std::error_code error;

            std::filesystem::remove_all(
                m_root,
                error);
        }

        TestWorkspace(
            const TestWorkspace&) = delete;

        TestWorkspace& operator=(
            const TestWorkspace&) = delete;

        [[nodiscard]]
        const std::filesystem::path&
            Root() const noexcept
        {
            return m_root;
        }

        [[nodiscard]]
        std::filesystem::path
            Desired() const
        {
            return m_root / "desired";
        }

        [[nodiscard]]
        std::filesystem::path
            Current() const
        {
            return m_root / "current";
        }

        [[nodiscard]]
        std::filesystem::path
            Manifest() const
        {
            return Desired() /
                "bootstrapforge.json";
        }

        void WriteManifest(
            std::string_view json) const
        {
            std::ofstream output(
                Manifest(),
                std::ios::binary |
                std::ios::trunc);

            if (!output)
            {
                throw std::runtime_error(
                    "Failed to create integration-test manifest.");
            }

            output << json;

            if (!output)
            {
                throw std::runtime_error(
                    "Failed to write integration-test manifest.");
            }
        }

    private:

        std::filesystem::path m_root;
    };

    void Require(
        bool condition,
        const std::string& message)
    {
        if (!condition)
        {
            std::cerr
                << "[FAIL] "
                << message
                << '\n';

            std::exit(
                EXIT_FAILURE);
        }
    }

    [[nodiscard]]
    std::size_t CountResourcesAtPaths(
        const Domain::Models::ArchitectureModel& model)
    {
        std::size_t count = 0;

        for (const auto& resource :
            model.Resources())
        {
            if (resource.Path().ToString() == "/src" ||
                resource.Path().ToString() == "/src/a" ||
                resource.Path().ToString() ==
                "/src/a/file.txt")
            {
                ++count;
            }
        }

        return count;
    }

    [[nodiscard]]
    bool HasResource(
        const Domain::Models::ArchitectureModel& model,
        std::string_view path)
    {
        for (const auto& resource :
            model.Resources())
        {
            if (resource.Path().ToString() == path)
            {
                return true;
            }
        }

        return false;
    }

    [[nodiscard]]
    const Domain::Models::Resource*
        FindResource(
            const Domain::Models::ArchitectureModel& model,
            std::string_view path)
    {
        for (const auto& resource :
            model.Resources())
        {
            if (resource.Path().ToString() == path)
            {
                return &resource;
            }
        }

        return nullptr;
    }

    void TestFullReconciliationPipeline()
    {
        TestWorkspace workspace;

        workspace.WriteManifest(
            R"json(
{
    "version": 1,
    "resources": [
        {
            "id": "src-directory",
            "path": "/src",
            "kind": "directory"
        },
        {
            "id": "src-a-directory",
            "path": "/src/a",
            "kind": "directory"
        },
        {
            "id": "source-file",
            "path": "/src/a/file.txt",
            "kind": "file",
            "content": "BootstrapForge integration test"
        }
    ],
    "relationships": []
}
)json");

        Infrastructure::Cryptography::Hashing::Blake3Hasher
            hasher;

        Application::Identity::ResourceFactory
            resourceFactory(
                hasher);

        Application::Identity::RelationshipFactory
            relationshipFactory(
                hasher);

        Application::Mapping::ManifestMapper
            mapper(
                resourceFactory,
                relationshipFactory);

        Infrastructure::Filesystem::ManifestFileReader
            fileReader;

        Infrastructure::Deserialization::
            JsonManifestDeserializer
            deserializer;

        Infrastructure::Loading::
            JsonDesiredArchitectureLoader
            loader(
                fileReader,
                deserializer,
                mapper);

        const auto desired =
            loader.Load(
                workspace.Manifest());

        Require(
            desired.Resources().size() == 3,
            "The desired architecture must contain three resources.");

        Require(
            HasResource(
                desired,
                "/src"),
            "The desired architecture must contain /src.");

        Require(
            HasResource(
                desired,
                "/src/a"),
            "The desired architecture must contain /src/a.");

        Require(
            HasResource(
                desired,
                "/src/a/file.txt"),
            "The desired architecture must contain /src/a/file.txt.");

        Infrastructure::Filesystem::
            PhysicalFilesystemScanner
            scanner;

        const auto initialCurrent =
            scanner.Scan(
                workspace.Current());

        Require(
            initialCurrent.Resources().empty(),
            "The integration-test current workspace must initially be empty.");

        Engine::Reconciliation::
            ArchitectureReconciler
            reconciler;

        const auto plan =
            reconciler.Reconcile(
                desired,
                initialCurrent);

        Require(
            plan.Operations().size() == 3,
            "Initial reconciliation must plan three resource creations.");

        Engine::Scheduler::
            ExecutionScheduler
            scheduler;

        const auto scheduledPlan =
            scheduler.Schedule(
                plan);

        Require(
            scheduledPlan.Operations().size() == 3,
            "Scheduled initial reconciliation must contain three operations.");

        Infrastructure::Execution::
            PhysicalExecutionBackend
            backend(
                workspace.Current());

        Engine::Execution::
            ExecutionExecutor
            executor;

        const auto result =
            executor.Execute(
                scheduledPlan,
                backend);

        Require(
            result.Success(),
            "Initial reconciliation execution must succeed.");

        Require(
            result.Total() == 3,
            "Initial reconciliation must execute three operations.");

        Require(
            result.Successful() == 3,
            "All three initial reconciliation operations must succeed.");

        Require(
            result.Failed() == 0,
            "Initial reconciliation must not contain failed operations.");

        Require(
            std::filesystem::is_directory(
                workspace.Current() / "src"),
            "The physical /src directory must exist.");

        Require(
            std::filesystem::is_directory(
                workspace.Current() / "src" / "a"),
            "The physical /src/a directory must exist.");

        Require(
            std::filesystem::is_regular_file(
                workspace.Current() /
                "src" /
                "a" /
                "file.txt"),
            "The physical /src/a/file.txt file must exist.");

        {
            std::ifstream input(
                workspace.Current() /
                "src" /
                "a" /
                "file.txt",
                std::ios::binary);

            Require(
                input.good(),
                "The integration-test file must be readable.");

            const std::string content{
                std::istreambuf_iterator<char>(input),
                std::istreambuf_iterator<char>()
            };

            Require(
                content ==
                "BootstrapForge integration test",
                "The physical file content must match the desired content.");
        }

        const auto reconciledCurrent =
            scanner.Scan(
                workspace.Current());

        Require(
            reconciledCurrent.Resources().size() == 3,
            "The scanner must discover exactly three resources after execution.");

        Require(
            CountResourcesAtPaths(
                reconciledCurrent) == 3,
            "The scanner must discover all desired resource paths.");

        const auto currentFile =
            FindResource(
                reconciledCurrent,
                "/src/a/file.txt");

        Require(
            currentFile != nullptr,
            "The scanner must discover /src/a/file.txt.");

        Require(
            currentFile->Content().has_value(),
            "The scanned file must contain file content.");

        Require(
            currentFile->Content()->Value() ==
            "BootstrapForge integration test",
            "The scanned file content must match the desired content.");

        const auto secondPlan =
            reconciler.Reconcile(
                desired,
                reconciledCurrent);

        Require(
            secondPlan.Operations().empty(),
            "A second reconciliation after convergence must produce zero operations.");

        const auto secondScheduledPlan =
            scheduler.Schedule(
                secondPlan);

        Require(
            secondScheduledPlan.Operations().empty(),
            "A converged architecture must produce an empty scheduled plan.");

        const auto secondResult =
            executor.Execute(
                secondScheduledPlan,
                backend);

        Require(
            secondResult.Success(),
            "Executing an empty converged plan must succeed.");

        Require(
            secondResult.Total() == 0,
            "The converged architecture must require zero executions.");
    }
}

int main()
{
    std::cout
        << "BootstrapForge Reconciliation integration tests\n";

    try
    {
        TestFullReconciliationPipeline();

        std::cout
            << "[PASS] FullReconciliationPipeline\n";

        std::cout
            << "All BootstrapForge reconciliation integration tests passed.\n";

        return EXIT_SUCCESS;
    }
    catch (const std::exception& exception)
    {
        std::cerr
            << "[FAIL] Unhandled exception: "
            << exception.what()
            << '\n';

        return EXIT_FAILURE;
    }
}