#pragma once

#include <BootstrapForge/Application/Identity/RelationshipFactory.hpp>
#include <BootstrapForge/Application/Identity/ResourceFactory.hpp>

#include <BootstrapForge/Domain/Models/ArchitectureModel.hpp>

#include <filesystem>

namespace BootstrapForge::Application::Discovery
{
    class FilesystemScanner final
    {
    public:

        explicit FilesystemScanner(
            Identity::ResourceFactory& resourceFactory,
            Identity::RelationshipFactory& relationshipFactory) noexcept;

        [[nodiscard]]
        Domain::Models::ArchitectureModel Scan(
            const std::filesystem::path& root) const;

    private:

        Identity::ResourceFactory& m_resourceFactory;
        Identity::RelationshipFactory& m_relationshipFactory;
    };

} // namespace BootstrapForge::Application::Discovery