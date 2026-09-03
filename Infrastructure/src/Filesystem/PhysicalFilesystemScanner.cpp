#include <BootstrapForge/Infrastructure/Filesystem/PhysicalFilesystemScanner.hpp>

#include <BootstrapForge/Application/Identity/ResourceFactory.hpp>

#include <BootstrapForge/Infrastructure/Cryptography/Hashing/Blake3Hasher.hpp>

#include <BootstrapForge/Domain/Models/Relationship.hpp>
#include <BootstrapForge/Domain/Types/Hash.hpp>
#include <BootstrapForge/Domain/Types/RelationshipId.hpp>
#include <BootstrapForge/Domain/Types/ResourceId.hpp>

#include <array>
#include <cstddef>
#include <filesystem>
#include <fstream>
#include <optional>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

namespace BootstrapForge::Infrastructure::Filesystem
{
    namespace
    {
        [[nodiscard]]
        int HexValue(
            char value) noexcept
        {
            if (value >= '0' && value <= '9')
            {
                return value - '0';
            }

            if (value >= 'a' && value <= 'f')
            {
                return value - 'a' + 10;
            }

            if (value >= 'A' && value <= 'F')
            {
                return value - 'A' + 10;
            }

            return -1;
        }

        [[nodiscard]]
        std::optional<Domain::Types::Hash>
            ParseHash(
                const std::string& value)
        {
            constexpr std::size_t ExpectedLength =
                Domain::Types::Hash::Size * 2;

            if (value.size() != ExpectedLength)
            {
                return std::nullopt;
            }

            std::array<
                std::byte,
                Domain::Types::Hash::Size>
                bytes{};

            for (std::size_t index = 0;
                index < Domain::Types::Hash::Size;
                ++index)
            {
                const int high =
                    HexValue(
                        value[index * 2]);

                const int low =
                    HexValue(
                        value[index * 2 + 1]);

                if (high < 0 || low < 0)
                {
                    return std::nullopt;
                }

                const unsigned int byteValue =
                    static_cast<unsigned int>(
                        (high << 4) | low);

                bytes[index] =
                    static_cast<std::byte>(
                        byteValue);
            }

            return Domain::Types::Hash(
                bytes);
        }

        [[nodiscard]]
        bool IsBootstrapForgeInternalPath(
            const std::filesystem::path& relative)
        {
            for (const auto& component : relative)
            {
                if (component ==
                    std::filesystem::path(
                        ".bootstrapforge"))
                {
                    return true;
                }
            }

            return false;
        }
    }

    std::optional<Domain::Models::ResourceKind>
        PhysicalFilesystemScanner::ToResourceKind(
            const std::filesystem::directory_entry& entry)
    {
        if (entry.is_symlink())
        {
            return Domain::Models::ResourceKind::SymbolicLink;
        }

        if (entry.is_directory())
        {
            return Domain::Models::ResourceKind::Directory;
        }

        if (entry.is_regular_file())
        {
            return Domain::Models::ResourceKind::File;
        }

        return std::nullopt;
    }

    std::filesystem::path
        PhysicalFilesystemScanner::RelationshipStore(
            const std::filesystem::path& root)
    {
        return
            root /
            ".bootstrapforge" /
            "relationships";
    }

    Domain::Models::ArchitectureModel
        PhysicalFilesystemScanner::LoadRelationships(
            const std::filesystem::path& root,
            std::vector<Domain::Models::Resource> resources)
    {
        std::vector<Domain::Models::Relationship>
            relationships;

        const std::filesystem::path store =
            RelationshipStore(root);

        if (!std::filesystem::exists(store))
        {
            return Domain::Models::ArchitectureModel(
                std::move(resources),
                std::move(relationships));
        }

        std::ifstream input(
            store);

        if (!input)
        {
            return Domain::Models::ArchitectureModel(
                std::move(resources),
                std::move(relationships));
        }

        std::string line;

        while (std::getline(
            input,
            line))
        {
            if (line.empty())
            {
                continue;
            }

            std::istringstream record(
                line);

            std::string relationshipId;
            std::string sourceId;
            std::string targetId;

            if (!(record
                >> relationshipId
                >> sourceId
                >> targetId))
            {
                continue;
            }

            //
            // Reject malformed records containing
            // unexpected additional fields.
            //

            std::string extra;

            if (record >> extra)
            {
                continue;
            }

            const auto relationshipHash =
                ParseHash(
                    relationshipId);

            const auto sourceHash =
                ParseHash(
                    sourceId);

            const auto targetHash =
                ParseHash(
                    targetId);

            if (!relationshipHash.has_value() ||
                !sourceHash.has_value() ||
                !targetHash.has_value())
            {
                continue;
            }

            relationships.emplace_back(
                Domain::Models::Relationship(
                    Domain::Types::RelationshipId(
                        *relationshipHash),

                    Domain::Types::ResourceId(
                        *sourceHash),

                    Domain::Types::ResourceId(
                        *targetHash)));
        }

        return Domain::Models::ArchitectureModel(
            std::move(resources),
            std::move(relationships));
    }

    Domain::Models::ArchitectureModel
        PhysicalFilesystemScanner::Scan(
            const std::filesystem::path& root) const
    {
        Cryptography::Hashing::Blake3Hasher hasher;

        Application::Identity::ResourceFactory
            resourceFactory(
                hasher);

        std::vector<Domain::Models::Resource>
            resources;

        if (!std::filesystem::exists(root))
        {
            return Domain::Models::ArchitectureModel(
                std::move(resources),
                {});
        }

        std::error_code iteratorError;

        std::filesystem::recursive_directory_iterator
            iterator(
                root,
                std::filesystem::directory_options::
                skip_permission_denied,
                iteratorError);

        const std::filesystem::
            recursive_directory_iterator end;

        while (iterator != end)
        {
            if (iteratorError)
            {
                iterator.increment(
                    iteratorError);

                continue;
            }

            const auto& entry =
                *iterator;

            std::error_code relativeError;

            const std::filesystem::path relative =
                std::filesystem::relative(
                    entry.path(),
                    root,
                    relativeError);

            if (relativeError)
            {
                iterator.increment(
                    iteratorError);

                continue;
            }

            //
            // .bootstrapforge is internal BootstrapForge
            // state and must not become a resource.
            //

            if (IsBootstrapForgeInternalPath(
                relative))
            {
                if (entry.is_directory())
                {
                    iterator.disable_recursion_pending();
                }

                iterator.increment(
                    iteratorError);

                continue;
            }

            const auto kind =
                ToResourceKind(entry);

            if (!kind.has_value())
            {
                iterator.increment(
                    iteratorError);

                continue;
            }

            //
            // Physical:
            //
            // C:\...\Workspace\current\src\hello.txt
            //
            // Relative:
            //
            // src\hello.txt
            //
            // Logical:
            //
            // /src/hello.txt
            //

            const std::string relativeLogical =
                relative.generic_string();

            const std::filesystem::path logicalPath =
                std::filesystem::path(
                    "/" + relativeLogical);

            std::optional<Domain::Values::FileContent>
                content =
                std::nullopt;

            if (*kind ==
                Domain::Models::ResourceKind::File)
            {
                std::ifstream input(
                    entry.path(),
                    std::ios::binary);

                if (!input)
                {
                    iterator.increment(
                        iteratorError);

                    continue;
                }

                std::ostringstream buffer;

                buffer << input.rdbuf();

                if (!input.good() &&
                    !input.eof())
                {
                    iterator.increment(
                        iteratorError);

                    continue;
                }

                const std::string value =
                    buffer.str();

                content =
                    Domain::Values::FileContent(
                        value);
            }

            resources.emplace_back(
                resourceFactory.Create(
                    logicalPath,
                    *kind,
                    std::move(content)));

            iterator.increment(
                iteratorError);
        }

        return LoadRelationships(
            root,
            std::move(resources));
    }

} // namespace BootstrapForge::Infrastructure::Filesystem