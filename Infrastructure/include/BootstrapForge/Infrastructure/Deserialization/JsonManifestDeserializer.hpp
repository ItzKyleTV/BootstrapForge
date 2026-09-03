#pragma once

#include <BootstrapForge/Application/Manifest/Manifest.hpp>

#include <string_view>

namespace BootstrapForge::Infrastructure::Deserialization
{
    class JsonManifestDeserializer final
    {
    public:

        [[nodiscard]]
        Application::Manifest::Manifest Deserialize(
            std::string_view json) const;
    };

} // namespace BootstrapForge::Infrastructure::Deserialization