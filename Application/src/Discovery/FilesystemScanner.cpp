#include <BootstrapForge/Application/Discovery/FilesystemScanner.hpp>

#include <unordered_map>
#include <utility>
#include <vector>

namespace BootstrapForge::Application::Discovery
{
    FilesystemScanner::FilesystemScanner(
        Identity::ResourceFactory& resourceFactory,
        Identity::RelationshipFactory& relationshipFactory) noexcept
        :
        m_resourceFactory(resourceFactory),
        m_relationshipFactory(relationshipFactory)
    {
    }

    Domain::Models::ArchitectureModel FilesystemScanner::Scan(
        const std::filesystem::path& root) const
    {
        using Domain::Models::Relationship;
        using Domain::Models::Resource;

        std::vector<Resource> resources;
        std::vector<Relationship> relationships;

        if (!std::filesystem::exists(root))
        {
            return Domain::Models::ArchitectureModel(
                std::move(resources),
                std::move(relationships));
        }

        std::unordered_map<std::filesystem::path, std::size_t> lookup;

        auto addResource =
            [&](const std::filesystem::path& path)
            {
                lookup.emplace(path, resources.size());
                resources.emplace_back(
                    m_resourceFactory.Create(path));
            };

        //
        // Pass 1
        // Discover every resource.
        //

        addResource(root);

        for (const auto& entry :
            std::filesystem::recursive_directory_iterator(root))
        {
            addResource(entry.path());
        }

        //
        // Pass 2
        // Discover parent-child relationships.
        //

        for (const auto& [path, childIndex] : lookup)
        {
            const auto parentPath = path.parent_path();

            const auto parent = lookup.find(parentPath);

            if (parent == lookup.end())
            {
                continue;
            }

            relationships.emplace_back(
                m_relationshipFactory.Create(
                    resources[parent->second],
                    resources[childIndex]));
        }

        return Domain::Models::ArchitectureModel(
            std::move(resources),
            std::move(relationships));
    }

} // namespace BootstrapForge::Application::Discovery