#include <BootstrapForge/Domain/Models/ArchitectureDelta.hpp>

#include <BootstrapForge/Engine/Differ/ArchitectureDiffer.hpp>

#include <BootstrapForge/Tests/TestSupport.hpp>

#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

namespace
{
    using BootstrapForge::Domain::Models::ArchitectureDelta;
    using BootstrapForge::Engine::Differ::ArchitectureDiffer;

    using namespace BootstrapForge::Tests;

    void TestIdenticalArchitectures()
    {
        const auto resource =
            MakeFile(
                1,
                "/item.txt",
                "hello");

        const auto desired =
            MakeModel(
                { resource });

        const auto current =
            MakeModel(
                { resource });

        const auto delta =
            ArchitectureDiffer()
            .Compare(
                desired,
                current);

        Require(
            delta.AddedResources().empty(),
            "Identical architectures must have no added resources.");

        Require(
            delta.RemovedResources().empty(),
            "Identical architectures must have no removed resources.");

        Require(
            delta.ModifiedResources().empty(),
            "Identical architectures must have no modified resources.");

        Require(
            delta.AddedRelationships().empty(),
            "Identical architectures must have no added relationships.");

        Require(
            delta.RemovedRelationships().empty(),
            "Identical architectures must have no removed relationships.");
    }

    void TestAddedResource()
    {
        const auto resource =
            MakeFile(
                1,
                "/item.txt",
                "hello");

        const auto desired =
            MakeModel(
                { resource });

        const auto current =
            MakeModel({});

        const auto delta =
            ArchitectureDiffer()
            .Compare(
                desired,
                current);

        Require(
            delta.AddedResources().size() == 1,
            "A resource present only in the desired model must be added.");

        Require(
            delta.AddedResources().front().Path() ==
            resource.Path(),
            "The added resource must preserve its desired path.");

        Require(
            delta.RemovedResources().empty(),
            "Adding a resource must not produce a removal.");

        Require(
            delta.ModifiedResources().empty(),
            "Adding a resource must not produce a modification.");
    }

    void TestRemovedResource()
    {
        const auto resource =
            MakeFile(
                1,
                "/item.txt",
                "hello");

        const auto desired =
            MakeModel({});

        const auto current =
            MakeModel(
                { resource });

        const auto delta =
            ArchitectureDiffer()
            .Compare(
                desired,
                current);

        Require(
            delta.AddedResources().empty(),
            "Removing a resource must not produce an addition.");

        Require(
            delta.RemovedResources().size() == 1,
            "A resource present only in the current model must be removed.");

        Require(
            delta.RemovedResources().front().Path() ==
            resource.Path(),
            "The removed resource must preserve its current path.");

        Require(
            delta.ModifiedResources().empty(),
            "Removing a resource must not produce a modification.");
    }

    void TestModifiedResource()
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

        const auto desired =
            MakeModel(
                { desiredResource });

        const auto current =
            MakeModel(
                { currentResource });

        const auto delta =
            ArchitectureDiffer()
            .Compare(
                desired,
                current);

        Require(
            delta.AddedResources().empty(),
            "A modified resource must not be classified as added.");

        Require(
            delta.RemovedResources().empty(),
            "A modified resource must not be classified as removed.");

        Require(
            delta.ModifiedResources().size() == 1,
            "A changed resource must be classified as modified.");

        Require(
            delta.ModifiedResources().front().Current().Content()
            == currentResource.Content(),
            "A modification must preserve the current resource state.");

        Require(
            delta.ModifiedResources().front().Desired().Content()
            == desiredResource.Content(),
            "A modification must preserve the desired resource state.");
    }

    void TestImplicitParentDirectory()
    {
        const auto currentDirectory =
            MakeResource(
                1,
                "src",
                "/src",
                ResourceKind::Directory);

        const auto desiredFile =
            MakeFile(
                2,
                "/src/file.txt",
                "hello");

        const auto desired =
            MakeModel(
                { desiredFile });

        const auto current =
            MakeModel(
                { currentDirectory });

        const auto delta =
            ArchitectureDiffer()
            .Compare(
                desired,
                current);

        Require(
            delta.RemovedResources().empty(),
            "A directory required by a desired descendant must not be removed.");
    }
}

int main()
{
    std::cout
        << "BootstrapForge ArchitectureDiffer tests\n";

    TestIdenticalArchitectures();

    std::cout
        << "[PASS] IdenticalArchitectures\n";

    TestAddedResource();

    std::cout
        << "[PASS] AddedResource\n";

    TestRemovedResource();

    std::cout
        << "[PASS] RemovedResource\n";

    TestModifiedResource();

    std::cout
        << "[PASS] ModifiedResource\n";

    TestImplicitParentDirectory();

    std::cout
        << "[PASS] ImplicitParentDirectory\n";

    std::cout
        << "All ArchitectureDiffer tests passed.\n";

    return EXIT_SUCCESS;
}