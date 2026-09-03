#pragma once

#include <BootstrapForge/Application/Manifest/Manifest.hpp>

namespace BootstrapForge::Application::Validation
{
    class ManifestValidator final
    {
    public:

        void Validate(
            const Manifest::Manifest& manifest) const;
    };

} // namespace BootstrapForge::Application::Validation