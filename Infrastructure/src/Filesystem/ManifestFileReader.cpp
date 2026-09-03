#include <BootstrapForge/Infrastructure/Filesystem/ManifestFileReader.hpp>

#include <fstream>
#include <sstream>
#include <stdexcept>

namespace BootstrapForge::Infrastructure::Filesystem
{
    std::string ManifestFileReader::Read(
        const std::filesystem::path& manifestPath) const
    {
        std::ifstream stream(
            manifestPath,
            std::ios::binary);

        if (!stream)
        {
            throw std::runtime_error(
                "Unable to open manifest: " +
                manifestPath.string());
        }

        std::ostringstream buffer;

        buffer << stream.rdbuf();

        return buffer.str();
    }

} // namespace BootstrapForge::Infrastructure::Filesystem