#include <BootstrapForge/Application/Discovery/FilesystemScanner.hpp>

#include <vector>

namespace BootstrapForge::Application::Discovery
{
    using BootstrapForge::Domain::Models::ArchitectureModel;
    using BootstrapForge::Domain::Models::Relationship;
    using BootstrapForge::Domain::Models::Resource;

    FilesystemScanner::FilesystemScanner(
        Identity::ResourceFactory& resourceFactory) noexcept
        :
        m_resourceFactory(resourceFactory)
    {
    }

    ArchitectureModel FilesystemScanner::Scan(
        const std::filesystem::path& root) const
    {
        std::vector<Resource> resources;
        std::vector<Relationship> relationships;

        if (!std::filesystem::exists(root))
        {
            return {};
        }

        resources.emplace_back(
            m_resourceFactory.Create(root));

        for (const auto& entry :
            std::filesystem::recursive_directory_iterator(root))
        {
            resources.emplace_back(
                m_resourceFactory.Create(entry.path()));
        }

        return ArchitectureModel(
            std::move(resources),
            std::move(relationships));
    }
}