#pragma once

#include <filesystem>
#include <string>

namespace BootstrapForge::Infrastructure::Filesystem
{
    class ManifestFileReader final
    {
    public:

        [[nodiscard]]
        std::string Read(
            const std::filesystem::path& manifestPath) const;
    };

} // namespace BootstrapForge::Infrastructure::Filesystem