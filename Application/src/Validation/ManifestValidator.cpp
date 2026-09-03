#include <BootstrapForge/Application/Validation/ManifestValidator.hpp>

#include <format>
#include <stdexcept>
#include <unordered_set>

namespace BootstrapForge::Application::Validation
{
    void ManifestValidator::Validate(
        const Manifest::Manifest& manifest) const
    {
        if (manifest.Version != 1)
        {
            throw std::invalid_argument(
                std::format(
                    "Unsupported manifest version {}.",
                    manifest.Version));
        }

        std::unordered_set<std::string> paths;

        bool hasRoot = false;

        for (const auto& resource : manifest.Resources)
        {
            if (resource.Path.empty())
            {
                throw std::invalid_argument(
                    "Manifest contains a resource with an empty path.");
            }

            if (resource.Path.front() != '/')
            {
                throw std::invalid_argument(
                    std::format(
                        "Resource '{}' is not an absolute path.",
                        resource.Path));
            }

            if (!paths.insert(resource.Path).second)
            {
                throw std::invalid_argument(
                    std::format(
                        "Duplicate resource '{}'.",
                        resource.Path));
            }

            if (resource.Path == "/")
            {
                hasRoot = true;
            }
        }

        if (!hasRoot)
        {
            throw std::invalid_argument(
                "Manifest must contain the root resource '/'.");
        }
    }

} // namespace BootstrapForge::Application::Validation