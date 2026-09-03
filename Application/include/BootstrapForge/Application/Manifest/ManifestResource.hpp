#pragma once

#include <BootstrapForge/Application/Manifest/ManifestResourceKind.hpp>

#include <optional>
#include <string>

namespace BootstrapForge::Application::Manifest
{
    struct ManifestResource
    {
        std::string Path;

        ManifestResourceKind Kind =
            ManifestResourceKind::Directory;

        std::optional<std::string> Content;
    };

} // namespace BootstrapForge::Application::Manifest