#include <BootstrapForge/Infrastructure/Deserialization/JsonManifestDeserializer.hpp>

#include <BootstrapForge/Application/Manifest/Manifest.hpp>

#include <nlohmann/json.hpp>

#include <cstdint>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_set>
#include <utility>

namespace BootstrapForge::Infrastructure::Deserialization
{
    using Json = nlohmann::json;

    Application::Manifest::Manifest
        JsonManifestDeserializer::Deserialize(
            std::string_view json) const
    {
        const Json document =
            Json::parse(json);

        Application::Manifest::Manifest manifest;

        //
        // Version
        //

        if (document.contains("version"))
        {
            if (!document.at("version").is_number_unsigned())
            {
                throw std::runtime_error(
                    "Manifest 'version' must be an unsigned integer.");
            }

            manifest.Version =
                document.at("version")
                .get<std::uint32_t>();
        }

        //
        // Resources
        //

        if (document.contains("resources"))
        {
            const auto& resources =
                document.at("resources");

            if (!resources.is_array())
            {
                throw std::runtime_error(
                    "'resources' must be an array.");
            }

            std::unordered_set<std::string> manifestIds;

            manifestIds.reserve(resources.size());

            for (const auto& jsonResource : resources)
            {
                if (!jsonResource.is_object())
                {
                    throw std::runtime_error(
                        "Manifest resource must be an object.");
                }

                //
                // ID
                //

                if (!jsonResource.contains("id"))
                {
                    throw std::runtime_error(
                        "Manifest resource is missing 'id'.");
                }

                if (!jsonResource.at("id").is_string())
                {
                    throw std::runtime_error(
                        "Manifest resource 'id' must be a string.");
                }

                Application::Manifest::ManifestResource resource;

                resource.Id =
                    jsonResource.at("id")
                    .get<std::string>();

                if (resource.Id.empty())
                {
                    throw std::runtime_error(
                        "Manifest resource 'id' cannot be empty.");
                }

                if (!manifestIds.emplace(resource.Id).second)
                {
                    throw std::runtime_error(
                        "Duplicate manifest resource id: " +
                        resource.Id);
                }

                //
                // Path
                //

                if (!jsonResource.contains("path"))
                {
                    throw std::runtime_error(
                        "Manifest resource is missing 'path'.");
                }

                if (!jsonResource.at("path").is_string())
                {
                    throw std::runtime_error(
                        "Manifest resource 'path' must be a string.");
                }

                resource.Path =
                    jsonResource.at("path")
                    .get<std::string>();

                if (resource.Path.empty())
                {
                    throw std::runtime_error(
                        "Manifest resource 'path' cannot be empty.");
                }

                //
                // Kind
                //

                if (!jsonResource.contains("kind"))
                {
                    throw std::runtime_error(
                        "Manifest resource is missing 'kind'.");
                }

                if (!jsonResource.at("kind").is_string())
                {
                    throw std::runtime_error(
                        "Manifest resource 'kind' must be a string.");
                }

                const std::string kind =
                    jsonResource.at("kind")
                    .get<std::string>();

                if (kind == "directory")
                {
                    resource.Kind =
                        Application::Manifest::ManifestResourceKind::Directory;
                }
                else if (kind == "file")
                {
                    resource.Kind =
                        Application::Manifest::ManifestResourceKind::File;
                }
                else if (kind == "symbolic-link")
                {
                    resource.Kind =
                        Application::Manifest::ManifestResourceKind::SymbolicLink;
                }
                else
                {
                    throw std::runtime_error(
                        "Unknown manifest resource kind: " +
                        kind);
                }

                //
                // Optional content
                //

                if (jsonResource.contains("content"))
                {
                    if (!jsonResource.at("content").is_string())
                    {
                        throw std::runtime_error(
                            "Manifest resource 'content' must be a string.");
                    }

                    resource.Content =
                        jsonResource.at("content")
                        .get<std::string>();

                    resource.HasContent = true;
                }

                manifest.Resources.emplace_back(
                    std::move(resource));
            }
        }

        //
        // Relationships
        //

        if (document.contains("relationships"))
        {
            const auto& relationships =
                document.at("relationships");

            if (!relationships.is_array())
            {
                throw std::runtime_error(
                    "'relationships' must be an array.");
            }

            std::unordered_set<std::string> relationshipIds;

            relationshipIds.reserve(
                relationships.size());

            //
            // Build the resource-ID set first.
            //
            // Relationships refer to resources by their
            // manifest IDs.
            //

            std::unordered_set<std::string> resourceIds;

            resourceIds.reserve(
                manifest.Resources.size());

            for (const auto& resource :
                manifest.Resources)
            {
                resourceIds.emplace(
                    resource.Id);
            }

            for (const auto& jsonRelationship :
                relationships)
            {
                if (!jsonRelationship.is_object())
                {
                    throw std::runtime_error(
                        "Manifest relationship must be an object.");
                }

                //
                // Relationship ID
                //

                if (!jsonRelationship.contains("id"))
                {
                    throw std::runtime_error(
                        "Manifest relationship is missing 'id'.");
                }

                if (!jsonRelationship.at("id").is_string())
                {
                    throw std::runtime_error(
                        "Manifest relationship 'id' must be a string.");
                }

                Application::Manifest::ManifestRelationship relationship;

                relationship.Id =
                    jsonRelationship.at("id")
                    .get<std::string>();

                if (relationship.Id.empty())
                {
                    throw std::runtime_error(
                        "Manifest relationship 'id' cannot be empty.");
                }

                if (!relationshipIds.emplace(
                    relationship.Id).second)
                {
                    throw std::runtime_error(
                        "Duplicate manifest relationship id: " +
                        relationship.Id);
                }

                //
                // Source
                //

                if (!jsonRelationship.contains("source"))
                {
                    throw std::runtime_error(
                        "Manifest relationship is missing 'source'.");
                }

                if (!jsonRelationship.at("source").is_string())
                {
                    throw std::runtime_error(
                        "Manifest relationship 'source' must be a string.");
                }

                relationship.Source =
                    jsonRelationship.at("source")
                    .get<std::string>();

                if (relationship.Source.empty())
                {
                    throw std::runtime_error(
                        "Manifest relationship 'source' cannot be empty.");
                }

                //
                // Target
                //

                if (!jsonRelationship.contains("target"))
                {
                    throw std::runtime_error(
                        "Manifest relationship is missing 'target'.");
                }

                if (!jsonRelationship.at("target").is_string())
                {
                    throw std::runtime_error(
                        "Manifest relationship 'target' must be a string.");
                }

                relationship.Target =
                    jsonRelationship.at("target")
                    .get<std::string>();

                if (relationship.Target.empty())
                {
                    throw std::runtime_error(
                        "Manifest relationship 'target' cannot be empty.");
                }

                //
                // Validate references.
                //

                if (!resourceIds.contains(
                    relationship.Source))
                {
                    throw std::runtime_error(
                        "Manifest relationship '" +
                        relationship.Id +
                        "' references unknown source resource: " +
                        relationship.Source);
                }

                if (!resourceIds.contains(
                    relationship.Target))
                {
                    throw std::runtime_error(
                        "Manifest relationship '" +
                        relationship.Id +
                        "' references unknown target resource: " +
                        relationship.Target);
                }

                manifest.Relationships.emplace_back(
                    std::move(relationship));
            }
        }

        return manifest;
    }

} // namespace BootstrapForge::Infrastructure::Deserialization