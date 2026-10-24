#pragma once

#include <BootstrapForge/Application/Generation/GenerationPlan.hpp>

#include <filesystem>
#include <optional>
#include <string>
#include <vector>

namespace BootstrapForge::Infrastructure::Generation
{
    struct CodePackageInfo
    {
        std::string Id;
        std::string Name;
        std::string Version;
        std::string Kind;
        std::string Description;
        std::vector<std::string> Dependencies;
    };

    struct LoadedCodePackage
    {
        CodePackageInfo Info;
        Application::Generation::GenerationPlan Plan;
    };

    class CodePackageLoader final
    {
    public:

        // Loads a local package manifest and its source files as a generation plan.
        // Package file contents are treated as data; no package scripts are run.
        [[nodiscard]]
        std::optional<LoadedCodePackage> Load(
            const std::filesystem::path& manifestPath,
            std::string& errorMessage) const;
    };

} // namespace BootstrapForge::Infrastructure::Generation
