#pragma once

#include <BootstrapForge/Application/Manifest/ManifestResource.hpp>

#include <cstdint>
#include <string>
#include <vector>

namespace BootstrapForge::Application::Manifest
{
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