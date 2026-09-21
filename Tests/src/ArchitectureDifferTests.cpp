#include <BootstrapForge/Domain/Models/ArchitectureDelta.hpp>
#include <BootstrapForge/Domain/Models/ArchitectureModel.hpp>
#include <BootstrapForge/Domain/Models/Resource.hpp>
#include <BootstrapForge/Domain/Models/ResourceKind.hpp>
#include <BootstrapForge/Domain/Types/Hash.hpp>
#include <BootstrapForge/Domain/Types/Name.hpp>
#include <BootstrapForge/Domain/Types/Path.hpp>
#include <BootstrapForge/Domain/Types/ResourceId.hpp>
#include <BootstrapForge/Domain/Values/FileContent.hpp>

#include <BootstrapForge/Engine/Differ/ArchitectureDiffer.hpp>

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
    using BootstrapForge::Domain::Models::ArchitectureModel;
    using BootstrapForge::Domain::Models::Resource;
    using BootstrapForge::Domain::Models::ResourceKind;

    using BootstrapForge::Domain::Types::Hash;
    using BootstrapForge::Domain::Types::Name;
    using BootstrapForge::Domain::Types::Path;
    using BootstrapForge::Domain::Types::ResourceId;

    using BootstrapForge::Domain::Values::FileContent;

    using BootstrapForge::Engine::Differ::ArchitectureDiffer;

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