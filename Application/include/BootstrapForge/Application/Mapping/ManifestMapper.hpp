#pragma once

#include <BootstrapForge/Application/Manifest/Manifest.hpp>

#include <BootstrapForge/Domain/Models/ArchitectureModel.hpp>
#include <BootstrapForge/Domain/Models/Relationship.hpp>
#include <BootstrapForge/Domain/Models/Resource.hpp>
#include <BootstrapForge/Domain/Types/ResourceId.hpp>

#include <unordered_map>

namespace BootstrapForge::Application::Identity
{
    class ResourceFactory;
    class RelationshipFactory;
}

namespace BootstrapForge::Application::Mapping
{
    class ManifestMapper final
    {
    public:

        ManifestMapper(
            Identity::ResourceFactory& resourceFactory,
            Identity::RelationshipFactory& relationshipFactory);

        [[nodiscard]]
        Domain::Models::ArchitectureModel Map(
            const Manifest::Manifest& manifest) const;

    private:

        [[nodiscard]]
        Domain::Models::Resource MapResource(
            const Manifest::ManifestResource& resource) const;

        [[nodiscard]]
        Domain::Models::Relationship MapRelationship(
            const Manifest::ManifestRelationship& relationship,
            const std::unordered_map<
            std::string,
            Domain::Types::ResourceId>& resourceIds) const;

        Identity::ResourceFactory& m_resourceFactory;
        Identity::RelationshipFactory& m_relationshipFactory;
    };

} // namespace BootstrapForge::Application::Mapping