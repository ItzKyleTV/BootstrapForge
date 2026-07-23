#pragma once

#include <BootstrapForge/Application/Identity/ResourceFactory.hpp>

#include <BootstrapForge/Domain/Models/ArchitectureModel.hpp>

#include <filesystem>

namespace BootstrapForge::Application::Discovery
{
    class FilesystemScanner final
    {
    public:

        explicit FilesystemScanner(
            Identity::ResourceFactory& resourceFactory) noexcept;

        [[nodiscard]]
        Domain::Models::ArchitectureModel Scan(
            const std::filesystem::path& root) const;

    private:

        Identity::ResourceFactory& m_resourceFactory;
    };
}