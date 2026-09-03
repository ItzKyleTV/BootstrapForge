#pragma once

#include <BootstrapForge/Application/Manifest/ManifestResourceKind.hpp>

#include <optional>
#include <string>

namespace BootstrapForge::Application::Manifest
{
    struct ManifestResource
    {
        std::string Id;

        std::string Path;

        ManifestResourceKind Kind =
            ManifestResourceKind::Directory;

        std::optional<std::string> Content;

        bool HasContent = false;

        std::optional<std::string> Target;

        bool HasTarget = false;
    };

} // namespace BootstrapForge::Application::Manifest