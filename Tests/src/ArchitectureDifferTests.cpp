#include <BootstrapForge/Domain/Models/ArchitectureDelta.hpp>
#include <BootstrapForge/Domain/Models/ArchitectureModel.hpp>
#include <BootstrapForge/Domain/Models/Resource.hpp>
#include <BootstrapForge/Domain/Models/ResourceKind.hpp>
#include <BootstrapForge/Domain/Models/ResourceModification.hpp>

#include <BootstrapForge/Domain/Operations/ExecutionPlan.hpp>
#include <BootstrapForge/Domain/Operations/Operation.hpp>
#include <BootstrapForge/Domain/Operations/OperationKind.hpp>
#include <BootstrapForge/Domain/Operations/PlannedOperation.hpp>
#include <BootstrapForge/Domain/Operations/RelationshipOperation.hpp>

#include <BootstrapForge/Domain/Types/Hash.hpp>
#include <BootstrapForge/Domain/Types/Name.hpp>
#include <BootstrapForge/Domain/Types/Path.hpp>
#include <BootstrapForge/Domain/Types/ResourceId.hpp>

#include <BootstrapForge/Domain/Values/FileContent.hpp>
#include <BootstrapForge/Domain/Values/SymbolicLinkTarget.hpp>

#include <BootstrapForge/Engine/Differ/ArchitectureDiffer.hpp>
#include <BootstrapForge/Engine/Planner/ArchitecturePlanner.hpp>

#include <array>
#include <cstddef>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <string>
#include <utility>
#include <variant>
#include <vector>

namespace
{
    using BootstrapForge::Domain::Models::ArchitectureDelta;
    using BootstrapForge::Domain::Models::ArchitectureModel;
    using BootstrapForge::Domain::Models::Resource;
    using BootstrapForge::Domain::Models::ResourceKind;
    using BootstrapForge::Domain::Models::ResourceModification;

    using BootstrapForge::Domain::Operations::ExecutionPlan;
    using BootstrapForge::Domain::Operations::Operation;
    using BootstrapForge::Domain::Operations::OperationKind;
    using BootstrapForge::Domain::Operations::PlannedOperation;
    using BootstrapForge::Domain::Operations::RelationshipOperation;

    using BootstrapForge::Domain::Types::Hash;
    using BootstrapForge::Domain::Types::Name;
    using BootstrapForge::Domain::Types::Path;
    using BootstrapForge::Domain::Types::ResourceId;

    using BootstrapForge::Domain::Values::FileContent;
    using BootstrapForge::Domain::Values::SymbolicLinkTarget;

    using BootstrapForge::Engine::Differ::ArchitectureDiffer;
    using BootstrapForge::Engine::Planner::ArchitecturePlanner;

    [[nodiscard]]
    Hash MakeHash(
        unsigned char value)
    {
        std::array<
            std::byte,
            Hash::Size>
            bytes{};

        bytes.back() =
            std::byte{ value };

        return Hash(bytes);
    }

    [[nodiscard]]
    Resource MakeResource(
        unsigned char id,
        std::string name,
        std::string path,
        ResourceKind kind)
    {
        return Resource(
            ResourceId(
                MakeHash(id)),
            Name(
                std::move(name)),
            Path(
                std::filesystem::path(
                    path)),
            kind);
    }

    [[nodiscard]]
    Resource MakeFile(
        unsigned char id,
        std::string path,
        std::string content)
    {
        return Resource(
            ResourceId(
                MakeHash(id)),
            Name(
                std::filesystem::path(path)
                .filename()
                .string()),
            Path(
                std::filesystem::path(path)),
            ResourceKind::File,
            FileContent(
                std::move(content)));
    }

    [[nodiscard]]
    Resource MakeSymbolicLink(
        unsigned char id,
        std::string path,
        std::string target)
    {
        return Resource(
            ResourceId(
                MakeHash(id)),
            Name(
                std::filesystem::path(path)
                .filename()
                .string()),
            Path(
                std::filesystem::path(path)),
            ResourceKind::SymbolicLink,
            std::nullopt,
            SymbolicLinkTarget(
                std::filesystem::path(
                    target)));
    }

    [[nodiscard]]
    ArchitectureModel MakeModel(
        std::vector<Resource> resources)
    {
        return ArchitectureModel(
            std::move(resources),
            {});
    }

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

        Require(
            std::holds_alternative<Operation>(
                plan.Operations().front()),
            "An added resource must produce a resource operation.");

        const auto& operation =
            std::get<Operation>(
                plan.Operations().front());

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

        Require(
            std::holds_alternative<Operation>(
                plan.Operations().front()),
            "A removed resource must produce a resource operation.");

        const auto& operation =
            std::get<Operation>(
                plan.Operations().front());

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

        Require(
            std::holds_alternative<Operation>(
                plan.Operations().front()),
            "An ordinary modification must produce a resource operation.");

        const auto& operation =
            std::get<Operation>(
                plan.Operations().front());

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

        Require(
            std::holds_alternative<Operation>(
                plan.Operations()[0]),
            "The symbolic-link DELETE must be a resource operation.");

        Require(
            std::holds_alternative<Operation>(
                plan.Operations()[1]),
            "The symbolic-link CREATE must be a resource operation.");

        const auto& deleteOperation =
            std::get<Operation>(
                plan.Operations()[0]);

        const auto& createOperation =
            std::get<Operation>(
                plan.Operations()[1]);

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
        << "BootstrapForge ArchitectureDiffer and ArchitecturePlanner tests\n";

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
        << "\n";

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
        << "\nAll ArchitectureDiffer and ArchitecturePlanner tests passed.\n";

    return EXIT_SUCCESS;
}