#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace BootstrapForge::Application::Manifest
{
    enum class ManifestResourceKind
    {
        Directory,
        File,
        SymbolicLink
    };

    struct ManifestResource
    {
        std::string Id;
        std::string Path;
        ManifestResourceKind Kind;
        std::string Content;
        bool HasContent = false;
    };

    struct ManifestRelationship
    {
        std::string Id;
        std::string Source;
        std::string Target;
    };

    struct Manifest
    {
        std::uint32_t Version = 1;

        std::vector<ManifestResource> Resources;
        std::vector<ManifestRelationship> Relationships;
    };

} // namespace BootstrapForge::Application::Manifest