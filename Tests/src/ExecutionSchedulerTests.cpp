#include <BootstrapForge/Engine/Scheduler/ExecutionScheduler.hpp>

#include <BootstrapForge/Tests/TestSupport.hpp>

#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <string>

namespace
{
    using BootstrapForge::Engine::Scheduler::ExecutionScheduler;

    using namespace BootstrapForge::Tests;

    void TestSchedulerCreateHierarchy()
    {
        const auto root =
            MakeResource(
                1,
                "src",
                "/src",
                ResourceKind::Directory);

        const auto directory =
            MakeResource(
                2,
                "a",
                "/src/a",
                ResourceKind::Directory);

        const auto file =
            MakeFile(
                3,
                "/src/a/file.txt",
                "hello");

        ExecutionPlan plan =
            MakeExecutionPlan(
                {
                    Operation(
                        OperationKind::Create,
                        file),

                    Operation(
                        OperationKind::Create,
                        directory),

                    Operation(
                        OperationKind::Create,
                        root)
                });

        const auto scheduled =
            ExecutionScheduler()
            .Schedule(
                plan);

        Require(
            scheduled.Operations().size() == 3,
            "Creating a hierarchy must preserve all three operations.");

        const auto& first =
            RequireResourceOperation(
                scheduled.Operations()[0],
                "The first hierarchy operation must be a resource operation.");

        const auto& second =
            RequireResourceOperation(
                scheduled.Operations()[1],
                "The second hierarchy operation must be a resource operation.");

        const auto& third =
            RequireResourceOperation(
                scheduled.Operations()[2],
                "The third hierarchy operation must be a resource operation.");

        Require(
            first.Resource().Path() ==
            root.Path(),
            "The root directory must be created first.");

        Require(
            second.Resource().Path() ==
            directory.Path(),
            "The child directory must be created after its parent.");

        Require(
            third.Resource().Path() ==
            file.Path(),
            "The file must be created after its parent directory.");
    }

    void TestSchedulerDeleteHierarchy()
    {
        const auto root =
            MakeResource(
                1,
                "src",
                "/src",
                ResourceKind::Directory);

        const auto directory =
            MakeResource(
                2,
                "a",
                "/src/a",
                ResourceKind::Directory);

        const auto file =
            MakeFile(
                3,
                "/src/a/file.txt",
                "hello");

        ExecutionPlan plan =
            MakeExecutionPlan(
                {
                    Operation(
                        OperationKind::Delete,
                        root),

                    Operation(
                        OperationKind::Delete,
                        file),

                    Operation(
                        OperationKind::Delete,
                        directory)
                });

        const auto scheduled =
            ExecutionScheduler()
            .Schedule(
                plan);

        Require(
            scheduled.Operations().size() == 3,
            "Deleting a hierarchy must preserve all three operations.");

        const auto& first =
            RequireResourceOperation(
                scheduled.Operations()[0],
                "The first delete operation must be a resource operation.");

        const auto& second =
            RequireResourceOperation(
                scheduled.Operations()[1],
                "The second delete operation must be a resource operation.");

        const auto& third =
            RequireResourceOperation(
                scheduled.Operations()[2],
                "The third delete operation must be a resource operation.");

        Require(
            first.Resource().Path() ==
            file.Path(),
            "The deepest child must be deleted first.");

        Require(
            second.Resource().Path() ==
            directory.Path(),
            "The child directory must be deleted before its parent.");

        Require(
            third.Resource().Path() ==
            root.Path(),
            "The root directory must be deleted last.");
    }

    void TestSchedulerSymbolicLinkReplacement()
    {
        const auto symbolicLink =
            MakeSymbolicLink(
                1,
                "/item",
                "/target");

        ExecutionPlan plan =
            MakeExecutionPlan(
                {
                    Operation(
                        OperationKind::Create,
                        symbolicLink),

                    Operation(
                        OperationKind::Delete,
                        symbolicLink)
                });

        const auto scheduled =
            ExecutionScheduler()
            .Schedule(
                plan);

        Require(
            scheduled.Operations().size() == 2,
            "A symbolic-link replacement must contain DELETE and CREATE.");

        const auto& first =
            RequireResourceOperation(
                scheduled.Operations()[0],
                "The first symbolic-link replacement operation must be a resource operation.");

        const auto& second =
            RequireResourceOperation(
                scheduled.Operations()[1],
                "The second symbolic-link replacement operation must be a resource operation.");

        Require(
            first.Kind() ==
            OperationKind::Delete,
            "A symbolic-link replacement must delete before creating.");

        Require(
            second.Kind() ==
            OperationKind::Create,
            "A symbolic-link replacement must create after deleting.");

        Require(
            first.Resource().Path() ==
            symbolicLink.Path(),
            "The symbolic-link DELETE must preserve the link path.");

        Require(
            second.Resource().Path() ==
            symbolicLink.Path(),
            "The symbolic-link CREATE must preserve the link path.");
    }

    void TestSchedulerRelationshipCreation()
    {
        const auto source =
            MakeFile(
                1,
                "/source",
                "source");

        const auto target =
            MakeFile(
                2,
                "/target",
                "target");

        const auto relationship =
            MakeRelationship(
                3,
                1,
                2);

        ExecutionPlan plan =
            MakeExecutionPlan(
                {
                    RelationshipOperation(
                        OperationKind::AddRelationship,
                        relationship),

                    Operation(
                        OperationKind::Create,
                        target),

                    Operation(
                        OperationKind::Create,
                        source)
                });

        const auto scheduled =
            ExecutionScheduler()
            .Schedule(
                plan);

        Require(
            scheduled.Operations().size() == 3,
            "Relationship creation must preserve both resources and the relationship.");

        const auto& first =
            RequireResourceOperation(
                scheduled.Operations()[0],
                "The first relationship-creation operation must be a resource operation.");

        const auto& second =
            RequireResourceOperation(
                scheduled.Operations()[1],
                "The second relationship-creation operation must be a resource operation.");

        const auto& third =
            RequireRelationshipOperation(
                scheduled.Operations()[2],
                "The relationship must be scheduled after its resources.");

        Require(
            first.Resource().Id() ==
            source.Id(),
            "The relationship source must be created first.");

        Require(
            second.Resource().Id() ==
            target.Id(),
            "The relationship target must be created second.");

        Require(
            third.Kind() ==
            OperationKind::AddRelationship,
            "The final operation must add the relationship.");
    }

    void TestSchedulerRelationshipRemoval()
    {
        const auto source =
            MakeFile(
                1,
                "/source",
                "source");

        const auto target =
            MakeFile(
                2,
                "/target",
                "target");

        const auto relationship =
            MakeRelationship(
                3,
                1,
                2);

        ExecutionPlan plan =
            MakeExecutionPlan(
                {
                    Operation(
                        OperationKind::Delete,
                        target),

                    RelationshipOperation(
                        OperationKind::RemoveRelationship,
                        relationship),

                    Operation(
                        OperationKind::Delete,
                        source)
                });

        const auto scheduled =
            ExecutionScheduler()
            .Schedule(
                plan);

        Require(
            scheduled.Operations().size() == 3,
            "Relationship removal must preserve both deletes and the relationship removal.");

        const auto& first =
            RequireRelationshipOperation(
                scheduled.Operations()[0],
                "Relationship removal must occur before resource deletion.");

        Require(
            first.Kind() ==
            OperationKind::RemoveRelationship,
            "The relationship must be removed before resources are deleted.");
    }

    void TestSchedulerOperationPhases()
    {
        const auto deleted =
            MakeFile(
                1,
                "/deleted",
                "deleted");

        const auto updated =
            MakeFile(
                2,
                "/updated",
                "new");

        const auto created =
            MakeFile(
                3,
                "/created",
                "created");

        ExecutionPlan plan =
            MakeExecutionPlan(
                {
                    Operation(
                        OperationKind::Create,
                        created),

                    Operation(
                        OperationKind::Update,
                        updated),

                    Operation(
                        OperationKind::Delete,
                        deleted)
                });

        const auto scheduled =
            ExecutionScheduler()
            .Schedule(
                plan);

        Require(
            scheduled.Operations().size() == 3,
            "The scheduler must preserve all resource operations.");

        const auto& first =
            RequireResourceOperation(
                scheduled.Operations()[0],
                "The first phase operation must be a resource operation.");

        const auto& second =
            RequireResourceOperation(
                scheduled.Operations()[1],
                "The second phase operation must be a resource operation.");

        const auto& third =
            RequireResourceOperation(
                scheduled.Operations()[2],
                "The third phase operation must be a resource operation.");

        Require(
            first.Kind() ==
            OperationKind::Delete,
            "DELETE operations must execute before UPDATE operations.");

        Require(
            second.Kind() ==
            OperationKind::Update,
            "UPDATE operations must execute before CREATE operations.");

        Require(
            third.Kind() ==
            OperationKind::Create,
            "CREATE operations must execute after UPDATE operations.");
    }

    void TestSchedulerDeterministicCreateOrdering()
    {
        const auto beta =
            MakeFile(
                1,
                "/beta",
                "beta");

        const auto alpha =
            MakeFile(
                2,
                "/alpha",
                "alpha");

        ExecutionPlan plan =
            MakeExecutionPlan(
                {
                    Operation(
                        OperationKind::Create,
                        beta),

                    Operation(
                        OperationKind::Create,
                        alpha)
                });

        const auto scheduled =
            ExecutionScheduler()
            .Schedule(
                plan);

        Require(
            scheduled.Operations().size() == 2,
            "Deterministic scheduling must preserve both operations.");

        const auto& first =
            RequireResourceOperation(
                scheduled.Operations()[0],
                "The first deterministic operation must be a resource operation.");

        const auto& second =
            RequireResourceOperation(
                scheduled.Operations()[1],
                "The second deterministic operation must be a resource operation.");

        Require(
            first.Resource().Path() ==
            alpha.Path(),
            "Ready CREATE operations must be ordered lexically by path.");

        Require(
            second.Resource().Path() ==
            beta.Path(),
            "The second CREATE operation must follow lexical path ordering.");
    }

    void TestSchedulerRejectsRelationshipCycle()
    {
        const auto first =
            MakeFile(
                1,
                "/first",
                "first");

        const auto second =
            MakeFile(
                2,
                "/second",
                "second");

        const auto firstToSecond =
            MakeRelationship(
                3,
                1,
                2);

        const auto secondToFirst =
            MakeRelationship(
                4,
                2,
                1);

        ExecutionPlan plan =
            MakeExecutionPlan(
                {
                    Operation(
                        OperationKind::Create,
                        first),

                    Operation(
                        OperationKind::Create,
                        second),

                    RelationshipOperation(
                        OperationKind::AddRelationship,
                        firstToSecond),

                    RelationshipOperation(
                        OperationKind::AddRelationship,
                        secondToFirst)
                });

        bool threw =
            false;

        try
        {
            static_cast<void>(
                ExecutionScheduler()
                .Schedule(
                    plan));
        }
        catch (const std::runtime_error&)
        {
            threw = true;
        }

        Require(
            threw,
            "A cyclic relationship graph must be rejected by the scheduler.");
    }
}

int main()
{
    std::cout
        << "BootstrapForge ExecutionScheduler tests\n";

    TestSchedulerCreateHierarchy();

    std::cout
        << "[PASS] SchedulerCreateHierarchy\n";

    TestSchedulerDeleteHierarchy();

    std::cout
        << "[PASS] SchedulerDeleteHierarchy\n";

    TestSchedulerSymbolicLinkReplacement();

    std::cout
        << "[PASS] SchedulerSymbolicLinkReplacement\n";

    TestSchedulerRelationshipCreation();

    std::cout
        << "[PASS] SchedulerRelationshipCreation\n";

    TestSchedulerRelationshipRemoval();

    std::cout
        << "[PASS] SchedulerRelationshipRemoval\n";

    TestSchedulerOperationPhases();

    std::cout
        << "[PASS] SchedulerOperationPhases\n";

    TestSchedulerDeterministicCreateOrdering();

    std::cout
        << "[PASS] SchedulerDeterministicCreateOrdering\n";

    TestSchedulerRejectsRelationshipCycle();

    std::cout
        << "[PASS] SchedulerRejectsRelationshipCycle\n";

    std::cout
        << "All ExecutionScheduler tests passed.\n";

    return EXIT_SUCCESS;
}