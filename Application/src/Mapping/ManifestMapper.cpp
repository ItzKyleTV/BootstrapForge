#include <BootstrapForge/Application/Mapping/ManifestMapper.hpp>

#include <BootstrapForge/Application/Identity/RelationshipFactory.hpp>
#include <BootstrapForge/Application/Identity/ResourceFactory.hpp>

#include <BootstrapForge/Domain/Models/ResourceKind.hpp>
#include <BootstrapForge/Domain/Values/FileContent.hpp>

#include <stdexcept>
#include <unordered_map>
#include <utility>
#include <vector>

namespace BootstrapForge::Application::Mapping
{
    ManifestMapper::ManifestMapper(
        Identity::ResourceFactory& resourceFactory,
        Identity::RelationshipFactory& relationshipFactory)
        :
        m_resourceFactory(resourceFactory),
        m_relationshipFactory(relationshipFactory)
    {
    }

    Domain::Models::Resource
        ManifestMapper::MapResource(
            const Manifest::ManifestResource& resource) const
    {
        std::optional<
            Domain::Values::FileContent> content;

        if (resource.HasContent)
        {
            content.emplace(resource.Content);
        }

        Domain::Models::ResourceKind kind;

        switch (resource.Kind)
        {
        case Manifest::ManifestResourceKind::Directory:
            kind =
                Domain::Models::ResourceKind::Directory;
            break;

        case Manifest::ManifestResourceKind::File:
            kind =
                Domain::Models::ResourceKind::File;
            break;

        case Manifest::ManifestResourceKind::SymbolicLink:
            kind =
                Domain::Models::ResourceKind::SymbolicLink;
            break;

        default:
            throw std::runtime_error(
                "Unknown manifest resource kind.");
        }

        return m_resourceFactory.Create(
            resource.Id,
            resource.Path,
            kind,
            std::move(content));
    }

    Domain::Models::Relationship
        ManifestMapper::MapRelationship(
            const Manifest::ManifestRelationship& relationship,
            const std::unordered_map<
            std::string,
            Domain::Types::ResourceId>& resourceIds) const
    {
        const auto source =
            resourceIds.find(
                relationship.Source);

        if (source == resourceIds.end())
        {
            throw std::runtime_error(
                "Relationship '" +
                relationship.Id +
                "' references unknown source resource: " +
                relationship.Source);
        }

        const auto target =
            resourceIds.find(
                relationship.Target);

        if (target == resourceIds.end())
        {
            throw std::runtime_error(
                "Relationship '" +
                relationship.Id +
                "' references unknown target resource: " +
                relationship.Target);
        }

        return m_relationshipFactory.Create(
            relationship.Id,
            source->second,
            target->second);
    }

    Domain::Models::ArchitectureModel
        ManifestMapper::Map(
            const Manifest::Manifest& manifest) const
    {
        std::vector<
            Domain::Models::Resource> resources;

        resources.reserve(
            manifest.Resources.size());

        std::unordered_map<
            std::string,
            Domain::Types::ResourceId> resourceIds;

        resourceIds.reserve(
            manifest.Resources.size());

        for (const auto& manifestResource :
            manifest.Resources)
        {
            auto resource =
                MapResource(manifestResource);

            resourceIds.emplace(
                manifestResource.Id,
                resource.Id());

            resources.emplace_back(
                std::move(resource));
        }

        std::vector<
            Domain::Models::Relationship> relationships;

        relationships.reserve(
            manifest.Relationships.size());

        for (const auto& manifestRelationship :
            manifest.Relationships)
        {
            relationships.emplace_back(
                MapRelationship(
                    manifestRelationship,
                    resourceIds));
        }

        return Domain::Models::ArchitectureModel(
            std::move(resources),
            std::move(relationships));
    }

} // namespace BootstrapForge::Application::Mapping