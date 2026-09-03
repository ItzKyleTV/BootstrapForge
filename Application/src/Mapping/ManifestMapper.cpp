#include <BootstrapForge/Application/Mapping/ManifestMapper.hpp>

#include <BootstrapForge/Application/Identity/RelationshipFactory.hpp>
#include <BootstrapForge/Application/Identity/ResourceFactory.hpp>

#include <BootstrapForge/Domain/Models/ResourceKind.hpp>

#include <BootstrapForge/Domain/Values/FileContent.hpp>
#include <BootstrapForge/Domain/Values/SymbolicLinkTarget.hpp>

#include <filesystem>
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
        //
        // File content
        //

        std::optional<
            Domain::Values::FileContent> content;

        if (resource.HasContent)
        {
            if (!resource.Content.has_value())
            {
                throw std::runtime_error(
                    "Manifest resource '" +
                    resource.Id +
                    "' has HasContent set but no content value.");
            }

            content.emplace(
                *resource.Content);
        }

        //
        // Resource kind
        //

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

        //
        // Symbolic-link target
        //

        std::optional<
            Domain::Values::SymbolicLinkTarget>
            symbolicLinkTarget;

        if (resource.HasTarget)
        {
            if (!resource.Target.has_value())
            {
                throw std::runtime_error(
                    "Manifest resource '" +
                    resource.Id +
                    "' has HasTarget set but no target value.");
            }

            if (resource.Target->empty())
            {
                throw std::runtime_error(
                    "Manifest resource '" +
                    resource.Id +
                    "' has an empty symbolic-link target.");
            }

            symbolicLinkTarget.emplace(
                std::filesystem::path(
                    *resource.Target));
        }

        //
        // Resource-specific validation.
        //
        // The deserializer already validates these rules, but the
        // mapper also protects the domain boundary from malformed
        // Manifest DTOs constructed by another caller.
        //

        if (kind ==
            Domain::Models::ResourceKind::SymbolicLink)
        {
            if (!symbolicLinkTarget.has_value())
            {
                throw std::runtime_error(
                    "Symbolic-link resource '" +
                    resource.Id +
                    "' is missing a target.");
            }

            if (content.has_value())
            {
                throw std::runtime_error(
                    "Symbolic-link resource '" +
                    resource.Id +
                    "' cannot contain file content.");
            }
        }
        else
        {
            if (symbolicLinkTarget.has_value())
            {
                throw std::runtime_error(
                    "Only symbolic-link resources may contain a target.");
            }
        }

        if (content.has_value() &&
            kind != Domain::Models::ResourceKind::File)
        {
            throw std::runtime_error(
                "Only file resources may contain content.");
        }

        return m_resourceFactory.Create(
            resource.Id,
            resource.Path,
            kind,
            std::move(content),
            std::move(symbolicLinkTarget));
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
                MapResource(
                    manifestResource);

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