#include <BootstrapForge/Domain/Models/ArchitectureDelta.hpp>
#include <BootstrapForge/Domain/Models/ResourceModification.hpp>

#include <BootstrapForge/Engine/Planner/ArchitecturePlanner.hpp>

#include <BootstrapForge/Tests/TestSupport.hpp>

#include <cstdlib>
#include <iostream>
#include <string>

namespace
{
    using BootstrapForge::Domain::Models::ArchitectureDelta;
    using BootstrapForge::Domain::Models::ResourceModification;

    using BootstrapForge::Engine::Planner::ArchitecturePlanner;

    using namespace BootstrapForge::Tests;

    void TestPlannerAddedResource()
    {
        const auto resource =
            MakeFile(
                1,
                "/item.txt",
                "hello");

        ArchitectureDelta delta(
            { resource },
            {},
            {},
            {},
            {});

        const auto plan =
            ArchitecturePlanner()
            .Plan(delta);

        Require(
            plan.Operations().size() == 1,
            "Planning an added resource must produce exactly one operation.");

        const auto& operation =
            RequireResourceOperation(
                plan.Operations().front(),
                "An added resource must produce a resource operation.");

        Require(
            operation.Kind() ==
            OperationKind::Create,
            "An added resource must produce a CREATE operation.");

        Require(
            operation.Resource().Path() ==
            resource.Path(),
            "The CREATE operation must preserve the resource path.");
    }

    void TestPlannerRemovedResource()
    {
        const auto resource =
            MakeFile(
                1,
                "/item.txt",
                "hello");

        ArchitectureDelta delta(
            {},
            { resource },
            {},
            {},
            {});

        const auto plan =
            ArchitecturePlanner()
            .Plan(delta);

        Require(
            plan.Operations().size() == 1,
            "Planning a removed resource must produce exactly one operation.");

        const auto& operation =
            RequireResourceOperation(
                plan.Operations().front(),
                "A removed resource must produce a resource operation.");

        Require(
            operation.Kind() ==
            OperationKind::Delete,
            "A removed resource must produce a DELETE operation.");

        Require(
            operation.Resource().Path() ==
            resource.Path(),
            "The DELETE operation must preserve the resource path.");
    }

    void TestPlannerModifiedResource()
    {
        const auto currentResource =
            MakeFile(
                1,
                "/item.txt",
                "old");

        const auto desiredResource =
            MakeFile(
                1,
                "/item.txt",
                "new");

        ArchitectureDelta delta(
            {},
            {},
            {
                ResourceModification(
                    currentResource,
                    desiredResource)
            },
            {},
            {});

        const auto plan =
            ArchitecturePlanner()
            .Plan(delta);

        Require(
            plan.Operations().size() == 1,
            "Planning an ordinary modification must produce exactly one operation.");

        const auto& operation =
            RequireResourceOperation(
                plan.Operations().front(),
                "An ordinary modification must produce a resource operation.");

        Require(
            operation.Kind() ==
            OperationKind::Update,
            "An ordinary resource modification must produce an UPDATE operation.");

        Require(
            operation.Resource().Content()
            == desiredResource.Content(),
            "The UPDATE operation must contain the desired resource state.");
    }

    void TestPlannerModifiedSymbolicLink()
    {
        const auto currentResource =
            MakeSymbolicLink(
                1,
                "/item",
                "/old-target");

        const auto desiredResource =
            MakeSymbolicLink(
                1,
                "/item",
                "/new-target");

        ArchitectureDelta delta(
            {},
            {},
            {
                ResourceModification(
                    currentResource,
                    desiredResource)
            },
            {},
            {});

        const auto plan =
            ArchitecturePlanner()
            .Plan(delta);

        Require(
            plan.Operations().size() == 2,
            "A modified symbolic link must produce DELETE and CREATE operations.");

        const auto& deleteOperation =
            RequireResourceOperation(
                plan.Operations()[0],
                "The symbolic-link DELETE must be a resource operation.");

        const auto& createOperation =
            RequireResourceOperation(
                plan.Operations()[1],
                "The symbolic-link CREATE must be a resource operation.");

        Require(
            deleteOperation.Kind() ==
            OperationKind::Delete,
            "A modified symbolic link must delete the existing link first.");

        Require(
            createOperation.Kind() ==
            OperationKind::Create,
            "A modified symbolic link must create the desired link second.");

        Require(
            deleteOperation.Resource().Path() ==
            currentResource.Path(),
            "The symbolic-link DELETE must target the current link path.");

        Require(
            createOperation.Resource().Path() ==
            desiredResource.Path(),
            "The symbolic-link CREATE must target the desired link path.");

        Require(
            createOperation.Resource().Target()
            == desiredResource.Target(),
            "The symbolic-link CREATE must contain the desired target.");
    }
}

int main()
{
    std::cout
        << "BootstrapForge ArchitecturePlanner tests\n";

    TestPlannerAddedResource();

    std::cout
        << "[PASS] PlannerAddedResource\n";

    TestPlannerRemovedResource();

    std::cout
        << "[PASS] PlannerRemovedResource\n";

    TestPlannerModifiedResource();

    std::cout
        << "[PASS] PlannerModifiedResource\n";

    TestPlannerModifiedSymbolicLink();

    std::cout
        << "[PASS] PlannerModifiedSymbolicLink\n";

    std::cout
        << "All ArchitecturePlanner tests passed.\n";

    return EXIT_SUCCESS;
}