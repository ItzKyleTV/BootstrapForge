#pragma once

#include <BootstrapForge/Domain/Models/ArchitectureModel.hpp>

#include <filesystem>

namespace BootstrapForge::Contracts::Filesystem
{
    class IFilesystemScanner
    {
    public:

        virtual ~IFilesystemScanner() = default;

        [[nodiscard]]
        virtual Domain::Models::ArchitectureModel Scan(
            const std::filesystem::path& root) const = 0;
    };
}