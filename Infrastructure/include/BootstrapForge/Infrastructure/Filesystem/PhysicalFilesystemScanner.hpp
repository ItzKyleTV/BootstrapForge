#pragma once

#include <BootstrapForge/Contracts/Filesystem/IFilesystemScanner.hpp>

#include <BootstrapForge/Domain/Models/ArchitectureModel.hpp>
#include <BootstrapForge/Domain/Models/ResourceKind.hpp>

#include <filesystem>
#include <optional>
#include <vector>

namespace BootstrapForge::Infrastructure::Filesystem
{
    class PhysicalFilesystemScanner final
        : public Contracts::Filesystem::IFilesystemScanner
    {
    public:

        [[nodiscard]]
        Domain::Models::ArchitectureModel Scan(
            const std::filesystem::path& root) const override;

    private:

        [[nodiscard]]
        static std::optional<Domain::Models::ResourceKind>
            ToResourceKind(
                const std::filesystem::directory_entry& entry);

        [[nodiscard]]
        static std::filesystem::path
            RelationshipStore(
                const std::filesystem::path& root);

        [[nodiscard]]
        static Domain::Models::ArchitectureModel
            LoadRelationships(
                const std::filesystem::path& root,
                std::vector<Domain::Models::Resource> resources);
    };

} // namespace BootstrapForge::Infrastructure::Filesystem