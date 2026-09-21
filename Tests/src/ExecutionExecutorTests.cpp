#include <BootstrapForge/Engine/Execution/ExecutionExecutor.hpp>

#include <BootstrapForge/Tests/TestSupport.hpp>

#include <cstdlib>
#include <iostream>
#include <string>

namespace
{
    using BootstrapForge::Engine::Execution::ExecutionExecutor;

    using namespace BootstrapForge::Tests;

    void TestExecutorCreateDirectory()
    {
        const auto resource =
            MakeResource(
                1,
                "directory",
                "/directory",
                ResourceKind::Directory);

        const auto plan =
            MakeScheduledExecutionPlan(
                {
                    Operation(
                        OperationKind::Create,
                        resource)
                });

        RecordingExecutionBackend backend;

        ExecutionExecutor()
            .Execute(
                plan,
                backend);

        Require(
            backend.Calls().size() == 1,
            "Creating a directory must produce exactly one backend call.");

        Require(
            backend.Calls().front() ==
            "CreateDirectory:/directory",
            "CREATE directory must dispatch to CreateDirectory.");
    }

    void TestExecutorCreateFile()
    {
        const auto resource =
            MakeFile(
                1,
                "/file.txt",
                "hello");

        const auto plan =
            MakeScheduledExecutionPlan(
                {
                    Operation(
                        OperationKind::Create,
                        resource)
                });

        RecordingExecutionBackend backend;

        ExecutionExecutor()
            .Execute(
                plan,
                backend);

        Require(
            backend.Calls().size() == 1,
            "Creating a file must produce exactly one backend call.");

        Require(
            backend.Calls().front() ==
            "CreateFile:/file.txt",
            "CREATE file must dispatch to CreateFile.");
    }

    void TestExecutorCreateSymbolicLink()
    {
        const auto resource =
            MakeSymbolicLink(
                1,
                "/link",
                "/target");

        const auto plan =
            MakeScheduledExecutionPlan(
                {
                    Operation(
                        OperationKind::Create,
                        resource)
                });

        RecordingExecutionBackend backend;

        ExecutionExecutor()
            .Execute(
                plan,
                backend);

        Require(
            backend.Calls().size() == 1,
            "Creating a symbolic link must produce exactly one backend call.");

        Require(
            backend.Calls().front() ==
            "CreateSymbolicLink:/link",
            "CREATE symbolic link must dispatch to CreateSymbolicLink.");
    }

    void TestExecutorUpdateFile()
    {
        const auto resource =
            MakeFile(
                1,
                "/file.txt",
                "updated");

        const auto plan =
            MakeScheduledExecutionPlan(
                {
                    Operation(
                        OperationKind::Update,
                        resource)
                });

        RecordingExecutionBackend backend;

        ExecutionExecutor()
            .Execute(
                plan,
                backend);

        Require(
            backend.Calls().size() == 1,
            "Updating a file must produce exactly one backend call.");

        Require(
            backend.Calls().front() ==
            "UpdateFile:/file.txt",
            "UPDATE file must dispatch to UpdateFile.");
    }

    void TestExecutorRejectsDirectoryUpdate()
    {
        const auto resource =
            MakeResource(
                1,
                "directory",
                "/directory",
                ResourceKind::Directory);

        const auto plan =
            MakeScheduledExecutionPlan(
                {
                    Operation(
                        OperationKind::Update,
                        resource)
                });

        RecordingExecutionBackend backend;

        ExecutionExecutor()
            .Execute(
                plan,
                backend);

        Require(
            backend.Calls().empty(),
            "UPDATE directory must not call the execution backend.");
    }

    void TestExecutorRejectsSymbolicLinkUpdate()
    {
        const auto resource =
            MakeSymbolicLink(
                1,
                "/link",
                "/target");

        const auto plan =
            MakeScheduledExecutionPlan(
                {
                    Operation(
                        OperationKind::Update,
                        resource)
                });

        RecordingExecutionBackend backend;

        ExecutionExecutor()
            .Execute(
                plan,
                backend);

        Require(
            backend.Calls().empty(),
            "UPDATE symbolic link must not call the execution backend.");
    }

    void TestExecutorDeleteDirectory()
    {
        const auto resource =
            MakeResource(
                1,
                "directory",
                "/directory",
                ResourceKind::Directory);

        const auto plan =
            MakeScheduledExecutionPlan(
                {
                    Operation(
                        OperationKind::Delete,
                        resource)
                });

        RecordingExecutionBackend backend;

        ExecutionExecutor()
            .Execute(
                plan,
                backend);

        Require(
            backend.Calls().size() == 1,
            "Deleting a directory must produce exactly one backend call.");

        Require(
            backend.Calls().front() ==
            "DeleteDirectory:/directory",
            "DELETE directory must dispatch to DeleteDirectory.");
    }

    void TestExecutorDeleteFile()
    {
        const auto resource =
            MakeFile(
                1,
                "/file.txt",
                "hello");

        const auto plan =
            MakeScheduledExecutionPlan(
                {
                    Operation(
                        OperationKind::Delete,
                        resource)
                });

        RecordingExecutionBackend backend;

        ExecutionExecutor()
            .Execute(
                plan,
                backend);

        Require(
            backend.Calls().size() == 1,
            "Deleting a file must produce exactly one backend call.");

        Require(
            backend.Calls().front() ==
            "DeleteFile:/file.txt",
            "DELETE file must dispatch to DeleteFile.");
    }

    void TestExecutorDeleteSymbolicLink()
    {
        const auto resource =
            MakeSymbolicLink(
                1,
                "/link",
                "/target");

        const auto plan =
            MakeScheduledExecutionPlan(
                {
                    Operation(
                        OperationKind::Delete,
                        resource)
                });

        RecordingExecutionBackend backend;

        ExecutionExecutor()
            .Execute(
                plan,
                backend);

        Require(
            backend.Calls().size() == 1,
            "Deleting a symbolic link must produce exactly one backend call.");

        Require(
            backend.Calls().front() ==
            "DeleteSymbolicLink:/link",
            "DELETE symbolic link must dispatch to DeleteSymbolicLink.");
    }

    void TestExecutorAddRelationship()
    {
        const auto relationship =
            MakeRelationship(
                3,
                1,
                2);

        const auto plan =
            MakeScheduledExecutionPlan(
                {
                    RelationshipOperation(
                        OperationKind::AddRelationship,
                        relationship)
                });

        RecordingExecutionBackend backend;

        ExecutionExecutor()
            .Execute(
                plan,
                backend);

        Require(
            backend.Calls().size() == 1,
            "Adding a relationship must produce exactly one backend call.");

        Require(
            backend.Calls().front() ==
            "AddRelationship:" +
            relationship.Id().Value().ToHex(),
            "ADD_RELATIONSHIP must dispatch to AddRelationship.");
    }

    void TestExecutorRemoveRelationship()
    {
        const auto relationship =
            MakeRelationship(
                3,
                1,
                2);

        const auto plan =
            MakeScheduledExecutionPlan(
                {
                    RelationshipOperation(
                        OperationKind::RemoveRelationship,
                        relationship)
                });

        RecordingExecutionBackend backend;

        ExecutionExecutor()
            .Execute(
                plan,
                backend);

        Require(
            backend.Calls().size() == 1,
            "Removing a relationship must produce exactly one backend call.");

        Require(
            backend.Calls().front() ==
            "RemoveRelationship:" +
            relationship.Id().Value().ToHex(),
            "REMOVE_RELATIONSHIP must dispatch to RemoveRelationship.");
    }

    void TestExecutorRecordsBackendFailure()
    {
        const auto resource =
            MakeFile(
                1,
                "/file.txt",
                "hello");

        const auto plan =
            MakeScheduledExecutionPlan(
                {
                    Operation(
                        OperationKind::Create,
                        resource)
                });

        RecordingExecutionBackend backend;

        backend.CreateFileResult = false;

        const auto result =
            ExecutionExecutor()
            .Execute(
                plan,
                backend);

        Require(
            result.Entries().size() == 1,
            "A failed backend execution must produce one result entry.");

        Require(
            backend.Calls().size() == 1,
            "A failed CREATE must still invoke the backend exactly once.");
    }

    void TestExecutorRecordsBackendSuccess()
    {
        const auto resource =
            MakeFile(
                1,
                "/file.txt",
                "hello");

        const auto plan =
            MakeScheduledExecutionPlan(
                {
                    Operation(
                        OperationKind::Create,
                        resource)
                });

        RecordingExecutionBackend backend;

        const auto result =
            ExecutionExecutor()
            .Execute(
                plan,
                backend);

        Require(
            result.Entries().size() == 1,
            "A successful backend execution must produce one result entry.");

        Require(
            backend.Calls().size() == 1,
            "A successful CREATE must invoke the backend exactly once.");
    }
}

int main()
{
    std::cout
        << "BootstrapForge ExecutionExecutor tests\n";

    TestExecutorCreateDirectory();

    std::cout
        << "[PASS] ExecutorCreateDirectory\n";

    TestExecutorCreateFile();

    std::cout
        << "[PASS] ExecutorCreateFile\n";

    TestExecutorCreateSymbolicLink();

    std::cout
        << "[PASS] ExecutorCreateSymbolicLink\n";

    TestExecutorUpdateFile();

    std::cout
        << "[PASS] ExecutorUpdateFile\n";

    TestExecutorRejectsDirectoryUpdate();

    std::cout
        << "[PASS] ExecutorRejectsDirectoryUpdate\n";

    TestExecutorRejectsSymbolicLinkUpdate();

    std::cout
        << "[PASS] ExecutorRejectsSymbolicLinkUpdate\n";

    TestExecutorDeleteDirectory();

    std::cout
        << "[PASS] ExecutorDeleteDirectory\n";

    TestExecutorDeleteFile();

    std::cout
        << "[PASS] ExecutorDeleteFile\n";

    TestExecutorDeleteSymbolicLink();

    std::cout
        << "[PASS] ExecutorDeleteSymbolicLink\n";

    TestExecutorAddRelationship();

    std::cout
        << "[PASS] ExecutorAddRelationship\n";

    TestExecutorRemoveRelationship();

    std::cout
        << "[PASS] ExecutorRemoveRelationship\n";

    TestExecutorRecordsBackendFailure();

    std::cout
        << "[PASS] ExecutorRecordsBackendFailure\n";

    TestExecutorRecordsBackendSuccess();

    std::cout
        << "[PASS] ExecutorRecordsBackendSuccess\n";

    std::cout
        << "All ExecutionExecutor tests passed.\n";

    return EXIT_SUCCESS;
}