#include <BootstrapForge/Domain/Models/ArchitectureDelta.hpp>
#include <BootstrapForge/Domain/Models/Resource.hpp>
#include <BootstrapForge/Domain/Models/ResourceKind.hpp>
#include <BootstrapForge/Domain/Models/ResourceModification.hpp>

#include <BootstrapForge/Domain/Operations/Operation.hpp>
#include <BootstrapForge/Domain/Operations/OperationKind.hpp>
#include <BootstrapForge/Domain/Operations/PlannedOperation.hpp>

#include <BootstrapForge/Domain/Types/Hash.hpp>
#include <BootstrapForge/Domain/Types/Name.hpp>
#include <BootstrapForge/Domain/Types/Path.hpp>
#include <BootstrapForge/Domain/Types/ResourceId.hpp>

#include <BootstrapForge/Domain/Values/FileContent.hpp>
#include <BootstrapForge/Domain/Values/SymbolicLinkTarget.hpp>

#include <BootstrapForge/Engine/Planner/ArchitecturePlanner.hpp>

#include <array>
#include <cstddef>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

namespace
{
    using BootstrapForge::Domain::Models::ArchitectureDelta;
    using BootstrapForge::Domain::Models::Resource;
    using BootstrapForge::Domain::Models::ResourceKind;
    using BootstrapForge::Domain::Models::ResourceModification;

    using BootstrapForge::Domain::Operations::Operation;
    using BootstrapForge::Domain::Operations::OperationKind;

    using BootstrapForge::Domain::Types::Hash;
    using BootstrapForge::Domain::Types::Name;
    using BootstrapForge::Domain::Types::Path;
    using BootstrapForge::Domain::Types::ResourceId;

    using BootstrapForge::Domain::Values::FileContent;
    using BootstrapForge::Domain::Values::SymbolicLinkTarget;

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
    Resource MakeDirectory(
        unsigned char id,
        std::string path)
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
            ResourceKind::Directory);
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
                std::filesystem::path(target)));
    }

    [[nodiscard]]
    ArchitectureDelta MakeAddedDelta(
        Resource resource)
    {
        std::vector<Resource> addedResources;

        addedResources.emplace_back(
            std::move(resource));

        return ArchitectureDelta(
            std::move(addedResources),
            {},
            {},
            {},
            {});
    }

    [[nodiscard]]
    ArchitectureDelta MakeModifiedDelta(
        Resource current,
        Resource desired)
    {
        std::vector<ResourceModification>
            modifiedResources;

        modifiedResources.emplace_back(
            std::move(current),
            std::move(desired));

        return ArchitectureDelta(
            {},
            {},
            std::move(modifiedResources),
            {},
            {});
    }

    [[nodiscard]]
    ArchitectureDelta MakeRemovedDelta(
        Resource resource)
    {
        std::vector<Resource> removedResources;

        removedResources.emplace_back(
            std::move(resource));

        return ArchitectureDelta(
            {},
            std::move(removedResources),
            {},
            {},
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

    void TestAddedResourceCreatesCreateOperation()
    {
        const auto resource =
            MakeFile(
                1,
                "/item.txt",
                "hello");

        const auto delta =
            MakeAddedDelta(
                resource);

        const auto plan =
            ArchitecturePlanner()
            .Plan(delta);

        Require(
            plan.Operations().size() == 1,
            "An added resource must produce exactly one operation.");

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

    void TestModifiedFileCreatesUpdateOperation()
    {
        const auto current =
            MakeFile(
                1,
                "/item.txt",
                "old");

        const auto desired =
            MakeFile(
                1,
                "/item.txt",
                "new");

        const auto delta =
            MakeModifiedDelta(
                current,
                desired);

        const auto plan =
            ArchitecturePlanner()
            .Plan(delta);

        Require(
            plan.Operations().size() == 1,
            "A modified file must produce exactly one operation.");

        Require(
            std::holds_alternative<Operation>(
                plan.Operations().front()),
            "A modified file must produce a resource operation.");

        const auto& operation =
            std::get<Operation>(
                plan.Operations().front());

        Require(
            operation.Kind() ==
            OperationKind::Update,
            "A modified file must produce an UPDATE operation.");

        Require(
            operation.Resource().Content()->Value() ==
            "new",
            "The UPDATE operation must contain the desired file content.");
    }

    void TestRemovedResourceCreatesDeleteOperation()
    {
        const auto resource =
            MakeFile(
                1,
                "/item.txt",
                "hello");

        const auto delta =
            MakeRemovedDelta(
                resource);

        const auto plan =
            ArchitecturePlanner()
            .Plan(delta);

        Require(
            plan.Operations().size() == 1,
            "A removed resource must produce exactly one operation.");

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

    void TestModifiedSymbolicLinkCreatesDeleteAndCreate()
    {
        const auto current =
            MakeSymbolicLink(
                1,
                "/test-link",
                "/old-target");

        const auto desired =
            MakeSymbolicLink(
                1,
                "/test-link",
                "/new-target");

        const auto delta =
            MakeModifiedDelta(
                current,
                desired);

        const auto plan =
            ArchitecturePlanner()
            .Plan(delta);

        Require(
            plan.Operations().size() == 2,
            "A modified symbolic link must produce DELETE and CREATE operations.");

        Require(
            std::holds_alternative<Operation>(
                plan.Operations()[0]),
            "The symbolic-link replacement DELETE must be a resource operation.");

        Require(
            std::holds_alternative<Operation>(
                plan.Operations()[1]),
            "The symbolic-link replacement CREATE must be a resource operation.");

        const auto& deleteOperation =
            std::get<Operation>(
                plan.Operations()[0]);

        const auto& createOperation =
            std::get<Operation>(
                plan.Operations()[1]);

        Require(
            deleteOperation.Kind() ==
            OperationKind::Delete,
            "The first symbolic-link replacement operation must be DELETE.");

        Require(
            createOperation.Kind() ==
            OperationKind::Create,
            "The second symbolic-link replacement operation must be CREATE.");

        Require(
            deleteOperation.Resource().Path() ==
            current.Path(),
            "The symbolic-link DELETE must use the existing link path.");

        Require(
            createOperation.Resource().Path() ==
            desired.Path(),
            "The symbolic-link CREATE must use the desired link path.");

        Require(
            createOperation.Resource().HasTarget(),
            "The symbolic-link CREATE must preserve the desired target.");

        Require(
            createOperation.Resource().Target()->ToString() ==
            "/new-target",
            "The symbolic-link CREATE must contain the new target.");

        Require(
            deleteOperation.Resource().Target()->ToString() ==
            "/old-target",
            "The symbolic-link DELETE must preserve the current target.");
    }
}

int main()
{
    std::cout
        << "BootstrapForge ArchitecturePlanner tests\n";

    TestAddedResourceCreatesCreateOperation();

    std::cout
        << "[PASS] AddedResourceCreatesCreateOperation\n";

    TestModifiedFileCreatesUpdateOperation();

    std::cout
        << "[PASS] ModifiedFileCreatesUpdateOperation\n";

    TestRemovedResourceCreatesDeleteOperation();

    std::cout
        << "[PASS] RemovedResourceCreatesDeleteOperation\n";

    TestModifiedSymbolicLinkCreatesDeleteAndCreate();

    std::cout
        << "[PASS] ModifiedSymbolicLinkCreatesDeleteAndCreate\n";

    std::cout
        << "All ArchitecturePlanner tests passed.\n";

    return EXIT_SUCCESS;
}